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

class ChunkIO {
public:

	~ChunkIO();

	void StartThread();
	void StopThread();

	void RequestToSaveChunk(ChunkSaveData&& c);
	void RequestToLoadChunk(int32_t cx, int32_t cz);
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

	

	std::vector<ChunkSaveData> m_chunkLoadedResult;

	std::condition_variable m_threadCv;


	constexpr static uint32_t loadTasksBudget = 8;
	constexpr static uint32_t saveTasksBudget = 8;
};