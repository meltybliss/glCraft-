#pragma once
#include "Core/ChunkJob.h"
#include "World/Chunk.h"
#include "World/ChunkUtil.h"
#include "Debugs/DebugSettings.h"
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

	ChunkPipeline(World* w) : m_world(w) {}

	void SetWorldSeed(uint64_t seed) {
		m_terrainGen = std::make_unique<TerrainGenerator>(seed);
	}

	void StartWorkerThreads();
	void StopWorkerThreads();

	void EnqueueJob(ChunkJob&& job);

	bool PopFrontMeshResult(MeshChunkResult& out);
	bool PopFrontGenResult(GeneratedChunkResult& out);

	void SetStreamCenter(int32_t curCx, int32_t curCz);
	std::vector<uint64_t> CancelQueuedOutside_ChunkJob();
	ChunkPipelineDebugStats GetDebugStats();

	void SetResultReadyCallback(std::function<void()> callback) {

		m_resultReadyCallback = std::move(callback);
	}


private:
	void StartLoop();
	

	void RemoveQueuedMeshJob_NoLock(uint64_t targetKey);
	static uint64_t GetChunkDistance(
		int32_t cx,
		int32_t cz,
		int32_t centerCx,
		int32_t centerCz
	);
	bool IsOutsideUnloadDistance(int32_t cx, int32_t cz) const;
	bool IsJobOutsideUnloadDistance(const ChunkJob& job) const;
	static int GetJobStagePriority(JobType type);

private:

	int WorkerCount = 0;
	constexpr static int MAX_WORKER_COUNT = 5;

private:
	World* m_world = nullptr;
	std::unique_ptr<TerrainGenerator> m_terrainGen;

	std::vector<std::thread> m_workers;

	std::atomic<bool> runningWorker = false;

	std::mutex jobsMutex;
	std::mutex meshResultMutex;
	std::mutex genResultMutex;
	std::mutex buildingChunksMutex;

	std::condition_variable workerCv;

	std::deque<ChunkJob> m_jobQueue;
	std::deque<MeshChunkResult> m_meshChunkResult;
	std::deque<GeneratedChunkResult> m_genChunkResult;
	std::unordered_map<uint64_t, std::unique_ptr<Chunk>> m_buildingChunks;

	std::atomic<int32_t> m_curStreamCx = 0;
	std::atomic<int32_t> m_curStreamCz = 0;
	std::atomic<int> m_activeWorkers = 0;
	std::atomic<uint64_t> m_completedTasks = 0;
	std::atomic<uint64_t> m_busyTimeNs = 0;
	

	std::unordered_set<uint64_t> m_pendingMeshJobs_ChunkKeys;

	//size_t m_cancelScanedIndex = 0;

private:

	std::function<void()> m_resultReadyCallback;

private:

	void ProcessJob(ChunkJob&& job);
	
};
