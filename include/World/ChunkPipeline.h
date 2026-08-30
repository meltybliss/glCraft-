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

	void SetStreamCenter(ChunkCoord coord);
	std::vector<ChunkCoord> CancelQueuedOutside_ChunkJob();
	ChunkPipelineDebugStats GetDebugStats();

	void SetResultReadyCallback(std::function<void()> callback) {

		m_resultReadyCallback = std::move(callback);
	}


private:
	void StartLoop();
	

	void RemoveQueuedMeshJob_NoLock(ChunkCoord targetKey);
	static uint64_t GetChunkDistance(
		ChunkCoord coord,
		ChunkCoord center
	);
	bool IsOutsideUnloadDistance(ChunkCoord coord) const;
	bool IsJobOutsideUnloadDistance(const ChunkJob& job) const;
	static int GetJobStagePriority(JobType type);

private:

	int WorkerCount = 0;
	constexpr static int MAX_WORKER_COUNT = 2;

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
	std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash> m_buildingChunks;

	std::atomic<int64_t> m_curStreamCx = 0;
	std::atomic<int64_t> m_curStreamCz = 0;
	std::atomic<int> m_activeWorkers = 0;
	std::atomic<uint64_t> m_completedTasks = 0;
	std::atomic<uint64_t> m_busyTimeNs = 0;
	

	std::unordered_set<ChunkCoord, ChunkCoordHash> m_pendingMeshJobs_ChunkKeys;

	//size_t m_cancelScanedIndex = 0;

private:

	std::function<void()> m_resultReadyCallback;

private:

	void ProcessJob(ChunkJob&& job);
	
};
