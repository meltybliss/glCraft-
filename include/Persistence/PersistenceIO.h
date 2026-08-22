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
#include <unordered_set>
#include <vector>

class PersistenceIO {
public:

	void StartThread();
	void StopThread();

	void RequestToSaveChunk(ChunkSaveData&& c);
	void RequestToLoadChunk(int32_t cx, int32_t cz);
	void SetStreamCenterAndCancelOutsideLoads(
		int32_t cx,
		int32_t cz,
		int32_t unloadDistance
	);

	void SaveWorld(WorldSaveData&& data);
	std::optional<WorldSaveData> LoadWorld();

	bool CheckDataExistence(int32_t cx, int32_t cz) const;

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
	
	std::atomic<bool> threadRunning;

	std::mutex m_TaskMutex;
	std::mutex m_loadResultMutex;
	

	std::deque<ChunkSaveTask> m_chunkSaveTasks;
	std::deque<ChunkLoadTask> m_chunkLoadTasks;
	std::unordered_set<uint64_t> m_pendingLoadKeys;

	

	std::deque<ChunkSaveData> m_chunkLoadedResult;

	std::condition_variable m_threadCv;
	std::atomic<int32_t> m_streamCx = 0;
	std::atomic<int32_t> m_streamCz = 0;


	constexpr static uint32_t loadTasksBudget = 8;
	constexpr static uint32_t saveTasksBudget = 8;
};