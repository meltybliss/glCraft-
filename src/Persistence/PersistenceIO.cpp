#include "Persistence/PersistenceIO.h"
#include <iostream>

void PersistenceIO::StartThread() {

	if (threadRunning.load()) return;

	threadRunning.store(true);

	chunkIOThread = std::thread([this]() {

		StartThreadLoop();

	});


}


void PersistenceIO::StopThread() {

	threadRunning.store(false);

	m_threadCv.notify_all();

	if (chunkIOThread.joinable()) {

		chunkIOThread.join();

	}


}



void PersistenceIO::StartThreadLoop() {

	while (true) {//StopThreradしたときにsaveなどをすべて行ってからthreadを終了させたいのでtrueで回す

	
		{
			std::unique_lock<std::mutex> lock(m_TaskMutex);

			m_threadCv.wait(lock, [this]() {

				return !threadRunning.load()
					|| !m_chunkLoadTasks.empty()
					|| !m_chunkSaveTasks.empty();
			});



			if (!threadRunning.load()
				&& m_chunkLoadTasks.empty()
				&& m_chunkSaveTasks.empty())
			{
				break;
			}

		}
		

		ProcLoadTasks();
		ProcSaveTasks();

	}


}



void PersistenceIO::RequestToSaveChunk(ChunkSaveData&& data) {

	{
		std::lock_guard<std::mutex> lock(m_TaskMutex);

		m_chunkSaveTasks.push_back(
			ChunkSaveTask{
				.saveData = std::move(data)
			}
		);
	}
	m_threadCv.notify_one();
}


void PersistenceIO::RequestToLoadChunk(int32_t cx, int32_t cz) {

	{
		std::lock_guard<std::mutex> lock(m_TaskMutex);

		m_chunkLoadTasks.push_back(
			ChunkLoadTask{
				.cx = cx,
				.cz = cz
			}
		);
	}
	m_threadCv.notify_one();
}



bool PersistenceIO::Check_HasItTasks() {

	std::scoped_lock lock(
		m_TaskMutex
	);

	return !m_chunkLoadTasks.empty()
		|| !m_chunkSaveTasks.empty();
}



void PersistenceIO::ProcSaveTasks() {


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



void PersistenceIO::ProcLoadTasks() {

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


bool PersistenceIO::CheckDataExistence(int32_t cx, int32_t cz) const {

	return c_diskStorage.CheckDataExistence(cx, cz);

}


std::optional<ChunkSaveData> 
PersistenceIO::PopChunkLoadedResult() {

	ChunkSaveData data;

	{
		std::lock_guard<std::mutex> lock(m_loadResultMutex);
		if (m_chunkLoadedResult.empty()) {
			return std::nullopt;
		}

		data = std::move(m_chunkLoadedResult.front());

		m_chunkLoadedResult.pop_front();
	}
	


	return std::move(data);

}



void PersistenceIO::SaveWorld(WorldSaveData&& data) {

	bool ok = w_diskStorage.SaveToDisk(data);

	if (!ok) {
		std::cerr << "failed to save world\n";
	}

}


std::optional<WorldSaveData> PersistenceIO::LoadWorld() {

	std::optional<WorldSaveData> data =
		w_diskStorage.LoadFromDisk();

	return data;
}