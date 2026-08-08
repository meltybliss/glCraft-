#include "Persistence/ChunkIO.h"


ChunkIO::~ChunkIO() {

	StopThread();

}


void ChunkIO::StartThread() {

	if (threadRunning.load()) return;

	threadRunning.store(true);

	chunkIOThread = std::thread([this]() {

		StartThreadLoop();

	});


}


void ChunkIO::StopThread() {

	threadRunning.store(false);

	m_threadCv.notify_all();

	if (chunkIOThread.joinable()) {

		chunkIOThread.join();

	}


}



void ChunkIO::StartThreadLoop() {

	while (threadRunning.load()) {

		if (Check_HasItTasks()) {


			ProcLoadTasks();
			ProcSaveTasks();

		}
		else {
			{
				std::unique_lock<std::mutex> lock(m_TaskMutex);

				m_threadCv.wait(lock, [this]() {

					return !threadRunning.load()
						|| !m_chunkLoadTasks.empty()
						|| !m_chunkSaveTasks.empty();
				});
			}
		}

		if (!threadRunning.load()) break;


	}


}



void ChunkIO::RequestToSaveChunk(ChunkSaveData&& data) {


	std::lock_guard<std::mutex> lock(m_TaskMutex);

	m_chunkSaveTasks.push_back(
		ChunkSaveTask{
			.saveData = std::move(data)
		}
	);
	m_threadCv.notify_one();
}


void ChunkIO::RequestToLoadChunk(int32_t cx, int32_t cz) {

	
	std::lock_guard<std::mutex> lock(m_TaskMutex);

	m_chunkLoadTasks.push_back(
		ChunkLoadTask{
			.cx = cx,
			.cz = cz
		}
	);
	m_threadCv.notify_one();
}



bool ChunkIO::Check_HasItTasks() {

	std::scoped_lock lock(
		m_TaskMutex
	);

	return !m_chunkLoadTasks.empty()
		|| !m_chunkSaveTasks.empty();
}



void ChunkIO::ProcSaveTasks() {


	std::vector<ChunkSaveTask> pendingResaveTasks;

	uint32_t budget = saveTasksBudget;

	while (budget > 0) {

		ChunkSaveTask task;
		{
			std::lock_guard<std::mutex> lock(m_TaskMutex);
			if (m_chunkSaveTasks.empty()) break;

			task = std::move(m_chunkSaveTasks.front());
			m_chunkSaveTasks.pop_front();

		}

		bool ok = c_diskStorage.SaveToDisk(task);

		budget--;

		if (!ok) {
			pendingResaveTasks.push_back(std::move(task));
		}

	}


	if (!pendingResaveTasks.empty())
	{
		std::lock_guard<std::mutex> lock(m_TaskMutex);

		for (auto& task : pendingResaveTasks)
		{

			m_chunkSaveTasks.push_back(
				std::move(task)
			);
		}
	}

}



void ChunkIO::ProcLoadTasks() {

	std::vector<ChunkLoadTask> pendingReloadTasks;

	uint32_t budget = loadTasksBudget;

	

	while (budget > 0) {
	
		ChunkLoadTask task;

		{
			std::lock_guard<std::mutex> lock(m_TaskMutex);

			if (m_chunkLoadTasks.empty()) break;

			task = std::move(m_chunkLoadTasks.front());

			m_chunkLoadTasks.pop_front();
		}


		budget--;

		ChunkDiskLoadResult result = 
			c_diskStorage.LoadFromDisk(task);

		switch (result.status)
		{
		case ChunkLoadStatus::Loaded:
			// 保存済みチャンクを使う

			{
				std::lock_guard<std::mutex> lock(m_loadResultMutex);

				m_chunkLoadedResult.push_back(std::move(*result.data));
			}


			continue;

		case ChunkLoadStatus::NotFound:
			
			continue;

		case ChunkLoadStatus::Corrupted:
			// 壊れているのでログ・復旧方針
			continue;

		case ChunkLoadStatus::IOError:
			
			pendingReloadTasks.push_back(std::move(task));

			continue;
		}


	}

	if (!pendingReloadTasks.empty()) {

		std::lock_guard<std::mutex> lock(m_TaskMutex);


		for (auto& task : pendingReloadTasks) {

			
			m_chunkLoadTasks.push_back(std::move(task));
		}
	}

}