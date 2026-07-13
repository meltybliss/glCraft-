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
#include <unordered_set>
#include <condition_variable>
#include <stdint.h>
#include <memory>
#include <functional>

using namespace ChunkUtil;

class WorldThread;
class World;

class ChunkPipeline {
public:
	explicit ChunkPipeline(World* w, uint64_t seed) : m_world(w), m_terrainGen(seed) {}

	void StartWorkerThread();
	void StopWorkerThread();

	void EnqueueJob(ChunkJob&& job);

	bool PopFrontMeshResult(MeshChunkResult& out);
	bool PopFrontGenResult(GeneratedChunkResult& out);

	void SetStreamCenter(const int64_t curCx, const int64_t curCz);
	std::vector<uint64_t> CancelQueuedOutside_ChunkJob();

	void SetResultReadyCallback(std::function<void()> callback) {

		m_resultReadyCallback = std::move(callback);
	}


	void RemoveQueuedMeshJob(uint64_t targetKey);

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
	std::mutex meshResultMutex;
	std::mutex genResultMutex;

	std::condition_variable workerCv;

	std::deque<ChunkJob> m_jobQueue;
	std::deque<MeshChunkResult> m_meshChunkResult;
	std::deque<GeneratedChunkResult> m_genChunkResult;
	std::unordered_map<uint64_t, std::unique_ptr<Chunk>> m_buildingChunks;

	std::atomic<int32_t> m_curStreamCx = 0;
	std::atomic<int32_t> m_curStreamCz = 0;
	

	std::unordered_set<uint64_t> m_pendingMeshJobs_ChunkKeys;

	//size_t m_cancelScanedIndex = 0;

private:

	std::function<void()> m_resultReadyCallback;

private:

	void ProcessJob(ChunkJob&& job);
	
};