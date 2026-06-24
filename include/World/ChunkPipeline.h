#pragma once
#include "Core/ChunkJob.h"
#include "World/Chunk.h"
#include "World/ChunkUtil.h"
#include "World/ChunkMeshSnapshot.h"
#include "World/ChunkResult.h"
#include "World/TerrainGenerator.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <deque>
#include <unordered_map>
#include <condition_variable>
#include <stdint.h>
#include <memory>

using namespace ChunkUtil;

class World;

class ChunkPipeline {
public:
	explicit ChunkPipeline(World* w, uint64_t seed) : m_world(w), m_terrainGen(seed) {}

	void StartWorkerThread();
	void StopWorkerThread();

	void EnqueueJob(ChunkJob&& job);

	bool PopFrontResult(ChunkResult& out);

	void SetStreamCenter(const int64_t curCx, const int64_t curCz);
	std::vector<uint64_t> CancelQueuedOutside_ChunkJob();

	//bool cancelPending = false;
private:
	void StartLoop();

private:

	static constexpr int MAX_CHUNK_CREATE_PER_TICK = 7;
	static constexpr int MAX_CHUNK_DESTROY_PER_TICK = 8;

	static constexpr int MAX_CHUNK_TERRAIN_PER_TICK = 7;
	static constexpr int MAX_CHUNK_MESH_PER_TICK = 8;


	static constexpr int JOB_CANCEL_BUDGET = 8;
private:
	World* m_world = nullptr;
	TerrainGenerator m_terrainGen;

	std::thread workerThread;
	std::atomic<bool> runningWorker = false;

	std::mutex jobsMutex;
	std::mutex resultMutex;

	std::condition_variable workerCv;

	std::deque<ChunkJob> m_jobQueue;
	std::deque<ChunkResult> m_chunkResult;
	std::unordered_map<uint64_t, std::unique_ptr<Chunk>> m_buildingChunks;

	std::atomic<int32_t> m_curStreamCx = 0;
	std::atomic<int32_t> m_curStreamCz = 0;


	//size_t m_cancelScanedIndex = 0;

private:

	void ProcessJob(ChunkJob&& job);
	
};