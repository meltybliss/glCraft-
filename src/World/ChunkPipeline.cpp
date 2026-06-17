#include "World/ChunkPipeline.h"
#include "World/TerrainGenerator.h"
#include "Render/MeshBuilder.h"

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


void ChunkPipeline::StartLoop() {
	while (runningWorker) {

		{
			std::unique_lock<std::mutex> lock(workerMutex);

			workerCv.wait(lock, [this]() {
				return !runningWorker || !m_jobQueue.empty();
			});

			if (!runningWorker) {
				break;
			}


		}

		{
			std::lock_guard<std::mutex> lock(jobsMutex);

			ChunkJob targetJob = m_jobQueue.front();
			switch (targetJob.type) {
				case JobType::CREATE_CHUNK: {
					const int32_t& cx = targetJob.cx;
					const int32_t& cz = targetJob.cz;

					uint64_t key = Index(cx, cz);
					if (m_buildingChunks.find(key) != m_buildingChunks.end()) {//todo ä˘Ç…Ç†Ç¡ÇΩÇÁÇªÇÍÇè¡ÇµÇƒÇ©ÇÁÇ‚ÇÈÇÊÇ§Ç…Ç∑ÇÈ
						std::unique_ptr<Chunk> c = std::make_unique<Chunk>(cx, cz);

						c->cx = cx;
						c->cz = cz;

						m_buildingChunks[key] = std::move(c);

						ChunkJob newJob = std::move(targetJob);
						newJob.type = JobType::GENERATE_TERRAIN;

						m_jobQueue.push_back(std::move(newJob));
					}
					

				}
				case JobType::GENERATE_TERRAIN: {
					const int32_t& cx = targetJob.cx;
					const int32_t& cz = targetJob.cz;

					uint64_t key = Index(cx, cz);

					auto it = m_buildingChunks.find(key);
					if (it != m_buildingChunks.end()) {
						auto& c = it->second;

						TerrainGenerator::GenerateTerrain(*c);
					}
				}
				case JobType::BUILD_MESH: {
					const int32_t& cx = targetJob.cx;
					const int32_t& cz = targetJob.cz;

					uint64_t key = Index(cx, cz);

					auto it = m_buildingChunks.find(key);
					if (it != m_buildingChunks.end()) {
						auto& c = it->second;
						if (targetJob.snapshot) {
							MeshData data = MeshBuilder::BuildChunkMesh(targetJob.snapshot.value());
							std::unique_ptr<Chunk> chunk = std::move(it->second);

							m_buildingChunks.erase(it);

							{
								std::lock_guard<std::mutex> lock(resultMutex);

								m_chunkResult.push_back({ 
									std::move(chunk), 
									std::move(data) 
								});
							}
						}
					}
				}
			}


			m_jobQueue.pop_front();
		}

	}

}


void ChunkPipeline::StopWorkerThread() {
	runningWorker = false;

	workerCv.notify_all();
	if (workerThread.joinable()) {
		workerThread.join();
	}

}