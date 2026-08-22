#include "Persistence/PersistenceIO.h"
#include "World/ChunkUtil.h"
#include <algorithm>
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
		const uint64_t key = ChunkUtil::Index(cx, cz);

		if (m_pendingLoadKeys.contains(key)) return;

		m_pendingLoadKeys.insert(key);

		m_chunkLoadTasks.push_back(
			ChunkLoadTask{
				.cx = cx,
				.cz = cz
			}
		);
	}
	m_threadCv.notify_one();
}


void PersistenceIO::SetStreamCenterAndCancelOutsideLoads(
	int32_t cx,
	int32_t cz,
	int32_t unloadDistance
) {
	{
		std::lock_guard<std::mutex> lock(m_TaskMutex);

		m_streamCx.store(cx);
		m_streamCz.store(cz);

		for (auto it = m_chunkLoadTasks.begin(); it != m_chunkLoadTasks.end();) {
			const int64_t dx = std::abs(
				static_cast<int64_t>(it->cx) - static_cast<int64_t>(cx)
			);
			const int64_t dz = std::abs(
				static_cast<int64_t>(it->cz) - static_cast<int64_t>(cz)
			);

			if (dx < unloadDistance && dz < unloadDistance) {
				++it;
				continue;
			}

			m_pendingLoadKeys.erase(ChunkUtil::Index(it->cx, it->cz));
			it = m_chunkLoadTasks.erase(it);
		}
	}

	{
		std::lock_guard<std::mutex> lock(m_loadResultMutex);

		std::erase_if(
			m_chunkLoadedResult,
			[cx, cz, unloadDistance](const ChunkSaveData& result) {
				const int64_t dx = std::abs(
					static_cast<int64_t>(result.cx) - cx
				);
				const int64_t dz = std::abs(
					static_cast<int64_t>(result.cz) - cz
				);

				return dx >= unloadDistance || dz >= unloadDistance;
			}
		);
	}
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
			const int64_t centerCx = m_streamCx.load();
			const int64_t centerCz = m_streamCz.load();

			auto nearestTask = std::min_element(
				m_chunkLoadTasks.begin(),
				m_chunkLoadTasks.end(),
				[centerCx, centerCz](const ChunkLoadTask& a, const ChunkLoadTask& b) {
					const int64_t distanceA = std::max(
						std::abs(static_cast<int64_t>(a.cx) - centerCx),
						std::abs(static_cast<int64_t>(a.cz) - centerCz)
					);
					const int64_t distanceB = std::max(
						std::abs(static_cast<int64_t>(b.cx) - centerCx),
						std::abs(static_cast<int64_t>(b.cz) - centerCz)
					);

					return distanceA < distanceB;
				}
			);

			task = std::move(*nearestTask);
			m_chunkLoadTasks.erase(nearestTask);
		}


		budget--;

		ChunkDiskLoadResult result = 
			c_diskStorage.LoadFromDisk(task);

		bool retryLoad = false;

		switch (result.status)
		{
		case ChunkLoadStatus::Loaded:
			// 保存済みチャンクを使う

			{
				std::lock_guard<std::mutex> lock(m_loadResultMutex);

				m_chunkLoadedResult.push_back(std::move(*result.data));
			}


			break;

		case ChunkLoadStatus::NotFound:
			
			break;

		case ChunkLoadStatus::Corrupted:
			// 壊れているのでログ・復旧方針
			break;

		case ChunkLoadStatus::IOError:
			
			pendingReloadTasks.push_back(std::move(task));
			retryLoad = true;

			break;
		}

		if (!retryLoad) {
			std::lock_guard<std::mutex> lock(m_TaskMutex);
			m_pendingLoadKeys.erase(ChunkUtil::Index(task.cx, task.cz));
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