#pragma once

#include "ChunkDiskStorage.h"
#include "WorldDiskStorage.h"
#include "ChunkSaveTask.h"
#include "ChunkLoadTask.h"
#include "World/Chunk.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class PersistenceIO {
public:

	void StartThread();
	void StopThread();

	void RequestToSaveChunk(ChunkSaveData&& c);
	void RequestToLoadChunk(ChunkCoord coord);
	void SetStreamCenterAndCancelOutsideLoads(
		ChunkCoord coord,
		int32_t unloadDistance
	);

	void SaveWorld(WorldSaveData&& data);
	std::optional<WorldSaveData> LoadWorld();
	bool ResetWorldStorage();

	bool CheckDataExistence(ChunkCoord coord);

	std::optional<ChunkSaveData> PopChunkLoadedResult();
private:

	void StartThreadLoop();


	void ProcSaveTasks();
	void ProcLoadTasks();

	bool Check_HasItTasks();
private:

	ChunkDiskStorage c_diskStorage;
	WorldDiskStorage w_diskStorage;

	std::thread chunkIOThread;
	
	std::atomic<bool> threadRunning = false;

	std::mutex m_TaskMutex;
	std::mutex m_loadResultMutex;
	

	std::deque<ChunkSaveTask> m_chunkSaveTasks;
	std::deque<ChunkLoadTask> m_chunkLoadTasks;
	std::unordered_set<ChunkCoord, ChunkCoordHash> m_pendingLoadKeys;
	std::unordered_map<ChunkCoord, std::size_t, ChunkCoordHash> m_pendingSaveCounts;

	

	std::deque<ChunkSaveData> m_chunkLoadedResult;

	std::condition_variable m_threadCv;
	ChunkCoord m_streamCoord{};


	constexpr static uint32_t loadTasksBudget = 8;
	constexpr static uint32_t saveTasksBudget = 8;
};
