#pragma once
#include "Core/ChunkJob.h"
#include "World/Chunk.h"
#include "World/ChunkKey.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <deque>
#include <unordered_map>
#include <condition_variable>
#include <stdint.h>
#include <memory>

using namespace ChunkKey;

class ChunkPipeline {
public:

	void StartWorkerThread();
	void StopWorkerThread();

private:
	void StartLoop();

private:

	static constexpr int MAX_CHUNK_CREATE_PER_TICK = 4;
	static constexpr int MAX_CHUNK_DESTROY_PER_TICK = 5;

	static constexpr int MAX_CHUNK_TERRAIN_PER_TICK = 5;
	static constexpr int MAX_CHUNK_MESH_PER_TICK = 5;

private:
	std::thread workerThread;
	std::atomic<bool> runningWorker = false;

	std::mutex workerMutex;
	std::mutex jobsMutex;

	std::condition_variable workerCv;

	std::deque<ChunkJob> m_jobQueue;
	std::unordered_map<uint64_t, std::unique_ptr<Chunk>> m_buildingChunks;
private:
};