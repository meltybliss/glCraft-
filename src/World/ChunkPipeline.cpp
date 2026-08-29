#include "World/ChunkPipeline.h"
#include "World/TerrainGenerator.h"
#include "Render/MeshBuilder.h"
#include "World/WorldThread.h"
#include "World/LightEngine.h"
#include <memory>
#include <iostream>
#include <algorithm>
#include "Util/ThreadSafeLogUtils.h"

void ChunkPipeline::StartWorkerThreads() {
	if (runningWorker) {
		return;
	}

	runningWorker = true;


	unsigned hw = std::thread::hardware_concurrency();

	WorkerCount = std::clamp<int>(
		static_cast<int>(hw) - 3,
		1,
		MAX_WORKER_COUNT
	);


	for (int i = 0; i < WorkerCount; ++i) {
		m_workers.emplace_back([this]() {

			StartLoop();
		});
	}


}




void ChunkPipeline::ProcessJob(ChunkJob&& targetJob) {

	switch (targetJob.type) {
		case JobType::CREATE_CHUNK: {
			const ChunkCoord coord = targetJob.coord;
			//todo 既にあったらそれを消してからやるようにする
			
			{
				std::lock_guard<std::mutex> lock(buildingChunksMutex);

				auto it = m_buildingChunks.find(coord);
				if (it != m_buildingChunks.end()) return;

				std::unique_ptr<Chunk> c = std::make_unique<Chunk>(coord);
				m_buildingChunks[coord] = std::move(c);

			}
				


			ChunkJob newJob = std::move(targetJob);
			newJob.type = JobType::GENERATE_TERRAIN;

			EnqueueJob(std::move(newJob));
			

			break;

		}
		case JobType::GENERATE_TERRAIN: {
			const ChunkCoord coord = targetJob.coord;

			std::unique_ptr<Chunk> chunk;

			{
				std::lock_guard<std::mutex> lock(buildingChunksMutex);


				auto it = m_buildingChunks.find(coord);
				if (it == m_buildingChunks.end()) return;

				chunk = std::move(it->second);
				m_buildingChunks.erase(it);
			}



			m_terrainGen->GenerateTerrain(*chunk);



			{
				std::lock_guard<std::mutex> lock(genResultMutex);

				m_genChunkResult.push_back({
					coord,
					std::move(chunk)
				});
			}



			if (m_resultReadyCallback) {
				m_resultReadyCallback();//wake
			}
				
			

			break;
		}


		
		case JobType::BUILD_MESH: {
			if (targetJob.snapshot) {
				MeshData data = MeshBuilder::BuildChunkMesh(*targetJob.snapshot);

				{
					std::lock_guard<std::mutex> lock(meshResultMutex);

					m_meshChunkResult.push_back({
						targetJob.coord,
						std::move(data)

					});
				}

				if (m_resultReadyCallback) {
					m_resultReadyCallback();//wake
				}


			}
			
			break;
		}
	}

}

void ChunkPipeline::StartLoop() {
	while (runningWorker) {


		ChunkJob targetJob;
		{
			std::unique_lock<std::mutex> lock(jobsMutex);

			workerCv.wait(lock, [this]() {
				return !runningWorker || !m_jobQueue.empty();
			});

			if (!runningWorker) {
				break;
			}
			const ChunkCoord center{m_curStreamCx.load(), m_curStreamCz.load()};

			//urgentなら最優先、urgentがないなら距離で優先度をつける、距離が同じならjobで優先度をつけます。
			auto bestJob = std::min_element(
				m_jobQueue.begin(),
				m_jobQueue.end(),
				[center](const ChunkJob& a, const ChunkJob& b) {
					if (a.urgent != b.urgent) {
						return a.urgent;
					}

					const uint64_t distanceA = GetChunkDistance(
						a.coord, center
					);
					const uint64_t distanceB = GetChunkDistance(
						b.coord, center
					);

					if (distanceA != distanceB) {
						return distanceA < distanceB;
					}

					return GetJobStagePriority(a.type) <
						GetJobStagePriority(b.type);
				}
			);

			targetJob = std::move(*bestJob);
			m_jobQueue.erase(bestJob);

			if (targetJob.type == JobType::BUILD_MESH) {
				m_pendingMeshJobs_ChunkKeys.erase(targetJob.coord);
			}

		}

		m_activeWorkers.fetch_add(1, std::memory_order_relaxed);
		const auto taskStart = std::chrono::steady_clock::now();

		ProcessJob(std::move(targetJob));

		const auto taskTime = std::chrono::steady_clock::now() - taskStart;
		const auto taskTimeNs = std::chrono::duration_cast<
			std::chrono::nanoseconds
		>(taskTime).count();

		m_busyTimeNs.fetch_add(
			static_cast<uint64_t>(taskTimeNs),
			std::memory_order_relaxed
		);
		m_completedTasks.fetch_add(1, std::memory_order_relaxed);
		m_activeWorkers.fetch_sub(1, std::memory_order_relaxed);

	}

}


void ChunkPipeline::StopWorkerThreads() {
	runningWorker = false;

	workerCv.notify_all();

	for (auto& worker : m_workers) {
		if (worker.joinable()) {
			worker.join();
		}
	}
}


ChunkPipelineDebugStats ChunkPipeline::GetDebugStats() {
	ChunkPipelineDebugStats stats;
	stats.workerCount = WorkerCount;
	stats.activeWorkers = m_activeWorkers.load(std::memory_order_relaxed);
	stats.completedTasks = m_completedTasks.load(std::memory_order_relaxed);
	stats.busyTimeNs = m_busyTimeNs.load(std::memory_order_relaxed);

	{
		std::lock_guard<std::mutex> lock(jobsMutex);
		stats.queuedTasks = m_jobQueue.size();

		for (const auto& job : m_jobQueue) {
			switch (job.type) {
			case JobType::CREATE_CHUNK:
				++stats.queuedCreateTasks;
				break;
			case JobType::GENERATE_TERRAIN:
				++stats.queuedTerrainTasks;
				break;
			case JobType::BUILD_MESH:
				++stats.queuedMeshTasks;
				break;
			}
		}
	}

	{
		std::lock_guard<std::mutex> lock(genResultMutex);
		stats.readyGenerateResults = m_genChunkResult.size();
	}

	{
		std::lock_guard<std::mutex> lock(meshResultMutex);
		stats.readyMeshResults = m_meshChunkResult.size();
	}

	return stats;
}


bool ChunkPipeline::PopFrontMeshResult(MeshChunkResult& out) {
	const ChunkCoord center{m_curStreamCx.load(), m_curStreamCz.load()};

	{
		std::lock_guard<std::mutex> lock(meshResultMutex);
		if (m_meshChunkResult.empty()) {
			return false;
		}

		auto nearestResult = std::min_element(
			m_meshChunkResult.begin(),
			m_meshChunkResult.end(),
			[center](
				const MeshChunkResult& a,
				const MeshChunkResult& b
			) {
				return GetChunkDistance(
					a.key,
					center
				) < GetChunkDistance(
					b.key,
					center
				);
			}
		);

		out = std::move(*nearestResult);
		m_meshChunkResult.erase(nearestResult);
	}

	return true;
}


bool ChunkPipeline::PopFrontGenResult(GeneratedChunkResult& out) {
	const ChunkCoord center{m_curStreamCx.load(), m_curStreamCz.load()};

	{
		std::lock_guard<std::mutex> lock(genResultMutex);

		if (m_genChunkResult.empty()) {
			return false;
		}

		auto nearestResult = std::min_element(
			m_genChunkResult.begin(),
			m_genChunkResult.end(),
			[center](
				const GeneratedChunkResult& a,
				const GeneratedChunkResult& b
			) {
				return GetChunkDistance(
					a.key,
					center
				) < GetChunkDistance(
					b.key,
					center
				);
			}
		);

		out = std::move(*nearestResult);
		m_genChunkResult.erase(nearestResult);
	}

	return true;

}


void ChunkPipeline::EnqueueJob(ChunkJob&& job) {



	const ChunkCoord key = job.coord;
	


	{
		std::lock_guard<std::mutex> lock(jobsMutex);

		if (job.type == JobType::BUILD_MESH) {
			if (m_pendingMeshJobs_ChunkKeys.contains(key)) {
				RemoveQueuedMeshJob_NoLock(key);
			}

			m_pendingMeshJobs_ChunkKeys.insert(key);
		}


		if (job.urgent) {
			m_jobQueue.push_front(std::move(job));
		}
		else {
			m_jobQueue.push_back(std::move(job));
		}
	}



	workerCv.notify_one();
}


void ChunkPipeline::SetStreamCenter(ChunkCoord coord) {
	std::lock_guard<std::mutex> lock(jobsMutex);
	m_curStreamCx.store(coord.x);
	m_curStreamCz.store(coord.z);
}


std::vector<ChunkCoord> ChunkPipeline::CancelQueuedOutside_ChunkJob() {
	std::vector<ChunkCoord> canceledKey;
	std::vector<ChunkCoord> canceledBuildingChunkKeys;

	{
		std::lock_guard<std::mutex> lock(jobsMutex);

		for (auto it = m_jobQueue.begin(); it != m_jobQueue.end();) {
			if (!IsJobOutsideUnloadDistance(*it)) {
				++it;
				continue;
			}

			const ChunkCoord key = it->coord;

			if (it->type == JobType::BUILD_MESH) {
				m_pendingMeshJobs_ChunkKeys.erase(key);
			}
			else {
				canceledKey.push_back(key);

				if (it->type == JobType::GENERATE_TERRAIN) {
					canceledBuildingChunkKeys.push_back(key);
				}
			}

			it = m_jobQueue.erase(it);
		}
	}

	if (!canceledBuildingChunkKeys.empty()) {
		std::lock_guard<std::mutex> lock(buildingChunksMutex);

		for (const ChunkCoord key : canceledBuildingChunkKeys) {
			m_buildingChunks.erase(key);
		}
	}

	{
		std::lock_guard<std::mutex> lock(meshResultMutex);

		std::erase_if(
			m_meshChunkResult,
			[this](const MeshChunkResult& result) {
				return IsOutsideUnloadDistance(result.key);
			}
		);
	}

	{
		std::lock_guard<std::mutex> lock(genResultMutex);

		for (auto it = m_genChunkResult.begin();
			it != m_genChunkResult.end();) {
			if (!IsOutsideUnloadDistance(it->key)) {
				++it;
				continue;
			}

			canceledKey.push_back(it->key);
			it = m_genChunkResult.erase(it);
		}
	}

	std::sort(canceledKey.begin(), canceledKey.end());
	canceledKey.erase(
		std::unique(canceledKey.begin(), canceledKey.end()),
		canceledKey.end()
	);


	return canceledKey;
}


uint64_t ChunkPipeline::GetChunkDistance(
	ChunkCoord coord,
	ChunkCoord center
) {
	const uint64_t dx = coord.x >= center.x
		? static_cast<uint64_t>(coord.x) - static_cast<uint64_t>(center.x)
		: static_cast<uint64_t>(center.x) - static_cast<uint64_t>(coord.x);
	const uint64_t dz = coord.z >= center.z
		? static_cast<uint64_t>(coord.z) - static_cast<uint64_t>(center.z)
		: static_cast<uint64_t>(center.z) - static_cast<uint64_t>(coord.z);

	return std::max(dx, dz);
}


bool ChunkPipeline::IsJobOutsideUnloadDistance(const ChunkJob& job) const {
	return IsOutsideUnloadDistance(job.coord);
}


bool ChunkPipeline::IsOutsideUnloadDistance(ChunkCoord coord) const {
	const int64_t dx = std::abs(
		coord.x - m_curStreamCx.load()
	);
	const int64_t dz = std::abs(
		coord.z - m_curStreamCz.load()
	);

	return dx >= WorldThread::Get_UNLOAD_DISTANCE() ||
		dz >= WorldThread::Get_UNLOAD_DISTANCE();
}


int ChunkPipeline::GetJobStagePriority(JobType type) {
	switch (type) {
	case JobType::BUILD_MESH:
		return 0;
	case JobType::GENERATE_TERRAIN:
		return 1;
	case JobType::CREATE_CHUNK:
		return 2;
	}

	return 3;
}


void ChunkPipeline::RemoveQueuedMeshJob_NoLock(ChunkCoord targetKey) {

	auto newEnd = std::remove_if(
		m_jobQueue.begin(),
		m_jobQueue.end(),
		[targetKey](const ChunkJob& job) {
			return job.type == JobType::BUILD_MESH &&
				job.coord == targetKey;
		}
	);

	m_jobQueue.erase(newEnd, m_jobQueue.end());
}

