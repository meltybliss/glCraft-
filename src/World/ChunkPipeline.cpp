#include "World/ChunkPipeline.h"
#include "World/TerrainGenerator.h"
#include "Render/MeshBuilder.h"
#include "World/World.h"
#include <memory>

void ChunkPipeline::StartWorkerThread() {
	if (runningWorker) {
		return;
	}

	runningWorker = true;

	workerThread = std::thread([this]() {
		
		StartLoop();

	});


}




void ChunkPipeline::ProcessJob(ChunkJob&& targetJob) {

	switch (targetJob.type) {
		case JobType::CREATE_CHUNK: {
			const int32_t& cx = targetJob.cx;
			const int32_t& cz = targetJob.cz;

			uint64_t key = Index(cx, cz);
			if (m_buildingChunks.find(key) == m_buildingChunks.end()) {//todo ä˘Ç…Ç†Ç¡ÇΩÇÁÇªÇÍÇè¡ÇµÇƒÇ©ÇÁÇ‚ÇÈÇÊÇ§Ç…Ç∑ÇÈ
				std::unique_ptr<Chunk> c = std::make_unique<Chunk>(cx, cz);

				c->cx = cx;
				c->cz = cz;

				m_buildingChunks[key] = std::move(c);

				ChunkJob newJob = std::move(targetJob);
				newJob.type = JobType::GENERATE_TERRAIN;

				EnqueueJob(std::move(newJob));
			}

			break;

		}
		case JobType::GENERATE_TERRAIN: {
			const int32_t& cx = targetJob.cx;
			const int32_t& cz = targetJob.cz;

			uint64_t key = Index(cx, cz);

			auto it = m_buildingChunks.find(key);
			if (it != m_buildingChunks.end()) {
				auto& c = it->second;

				m_terrainGen.GenerateTerrain(*c);


				ChunkJob newJob = std::move(targetJob);
				newJob.type = JobType::BUILD_MESH;
				newJob.meshSource = MeshBuildSource::INSTANCE_NEW_CHUNK;
				
				newJob.snapshot = m_world->CreateMeshSnapshot(*c);

				EnqueueJob(std::move(newJob));
			}

			break;
		}
		case JobType::BUILD_MESH: {
			const int32_t& cx = targetJob.cx;
			const int32_t& cz = targetJob.cz;

			uint64_t key = Index(cx, cz);

			if (targetJob.meshSource == MeshBuildSource::INSTANCE_NEW_CHUNK) {
				auto it = m_buildingChunks.find(key);
				if (it != m_buildingChunks.end()) {
					auto& c = it->second;
					if (targetJob.snapshot) {
						MeshData data = MeshBuilder::BuildChunkMesh(*targetJob.snapshot);
						std::unique_ptr<Chunk> chunk = std::move(it->second);

						m_buildingChunks.erase(it);

						{
							std::lock_guard<std::mutex> lock(resultMutex);

							m_chunkResult.push_back({
								key,
								std::move(data),
								std::move(chunk)

							});
						}
					}
				}
			}
			else if (targetJob.meshSource == MeshBuildSource::SNAPSHOT) {
				if (targetJob.snapshot) {
					MeshData data = MeshBuilder::BuildChunkMesh(*targetJob.snapshot);

					{
						std::lock_guard<std::mutex> lock(resultMutex);

						m_chunkResult.push_back({
							key,
							std::move(data)

						});
					}

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

			targetJob = std::move(m_jobQueue.front());
			m_jobQueue.pop_front();
		}

		ProcessJob(std::move(targetJob));

	}

}


void ChunkPipeline::StopWorkerThread() {
	runningWorker = false;

	workerCv.notify_all();
	if (workerThread.joinable()) {
		workerThread.join();
	}

}


bool ChunkPipeline::PopFrontResult(ChunkResult& out) {
	
	{
		std::lock_guard<std::mutex> lock(resultMutex);
		if (m_chunkResult.empty()) {
			return false;
		}

		out = std::move(m_chunkResult.front());
		m_chunkResult.pop_front();
	}

	return true;
}

void ChunkPipeline::EnqueueJob(ChunkJob&& job) {
	{
		std::lock_guard<std::mutex> lock(jobsMutex);

		if (job.urgent) {
			m_jobQueue.push_front(std::move(job));
		}
		else {
			m_jobQueue.push_back(std::move(job));
		}
	}

	workerCv.notify_all();
}


void ChunkPipeline::SetStreamCenter(const int64_t curCx, const int64_t curCz) {
	m_curStreamCx.store(curCx);
	m_curStreamCz.store(curCz);
}


std::vector<uint64_t> ChunkPipeline::CancelQueuedOutside_ChunkJob() {
	std::vector<uint64_t> canceledKey;

	std::lock_guard<std::mutex> lock(jobsMutex);
	if (m_jobQueue.empty()) return canceledKey;

	auto newEnd = std::remove_if(
		m_jobQueue.begin(),
		m_jobQueue.end(), 
		[&, this](const ChunkJob& job) {

			if (job.type != JobType::CREATE_CHUNK) return false;

			const int32_t cx = job.cx;
			const int32_t cz = job.cz;

			const int32_t dx = std::abs(m_curStreamCx.load() - cx);
			const int32_t dz = std::abs(m_curStreamCz.load() - cz);

			bool willCancel =
				dx >= World::Get_UNLOAD_DISTANCE() ||
				dz >= World::Get_UNLOAD_DISTANCE();
		
			if (willCancel) {
				canceledKey.push_back(Index(cx, cz));
			}

			return willCancel;
		}
	);

	m_jobQueue.erase(newEnd, m_jobQueue.end());


	return canceledKey;
}

/*
void ChunkPipeline::CancelQueuedOutside_ChunkJob() {
	if (!cancelPending) return;

	std::lock_guard<std::mutex> lock(jobsMutex);

	int budget = JOB_CANCEL_BUDGET;

	auto it = m_jobQueue.begin();
	std::advance(it, m_cancelScanedIndex);

	while (it != m_jobQueue.end() && budget > 0) {

		if (budget <= 0) break;

		const int32_t cx = it->cx;
		const int32_t cz = it->cz;

		const int32_t dx = std::abs(m_curStreamCx - cx);
		const int32_t dz = std::abs(m_curStreamCz - cz);

		bool shouldErase =
			dx >= World::Get_UNLOAD_DISTANCE() ||
			dz >= World::Get_UNLOAD_DISTANCE();

		if (shouldErase) {
			it = m_jobQueue.erase(it);
			--budget;
		}
		else {
			++it;
		}

		++m_cancelScanedIndex;
	}



}*/