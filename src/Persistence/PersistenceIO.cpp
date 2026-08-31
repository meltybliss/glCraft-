#include "Persistence/PersistenceIO.h"
#include "World/ChunkUtil.h"
#include <algorithm>
#include <iostream>
#include <limits>

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
		

		ProcSaveTasks();
		ProcLoadTasks();

	}


}



void PersistenceIO::RequestToSaveChunk(ChunkSaveData&& data) {
	const ChunkCoord coord = data.coord;

	{
		std::lock_guard<std::mutex> lock(m_TaskMutex);
		++m_pendingSaveCounts[coord];

		m_chunkSaveTasks.push_back(
			ChunkSaveTask{
				.saveData = std::move(data)
			}
		);
	}
	m_threadCv.notify_one();
}


void PersistenceIO::RequestToLoadChunk(ChunkCoord coord) {

	{
		std::lock_guard<std::mutex> lock(m_TaskMutex);
		if (m_pendingLoadKeys.contains(coord)) return;

		m_pendingLoadKeys.insert(coord);

		m_chunkLoadTasks.push_back(
			ChunkLoadTask{ .coord = coord }
		);
	}
	m_threadCv.notify_one();
}


void PersistenceIO::SetStreamCenterAndCancelOutsideLoads(
	ChunkCoord coord,
	int32_t unloadDistance
) {
	{
		std::lock_guard<std::mutex> lock(m_TaskMutex);

		m_streamCoord = coord;

		for (auto it = m_chunkLoadTasks.begin(); it != m_chunkLoadTasks.end();) {
			const int64_t dx = std::abs(
				it->coord.x - coord.x
			);
			const int64_t dz = std::abs(
				it->coord.z - coord.z
			);

			if (dx < unloadDistance && dz < unloadDistance) {
				++it;
				continue;
			}

			m_pendingLoadKeys.erase(it->coord);
			it = m_chunkLoadTasks.erase(it);
		}
	}

	{
		std::lock_guard<std::mutex> lock(m_loadResultMutex);

		std::erase_if(
			m_chunkLoadedResult,
			[coord, unloadDistance](const ChunkSaveData& result) {
				const int64_t dx = std::abs(
					result.coord.x - coord.x
				);
				const int64_t dz = std::abs(
					result.coord.z - coord.z
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
		else {
			std::lock_guard<std::mutex> lock(m_TaskMutex);
			auto pending = m_pendingSaveCounts.find(task.saveData.coord);
			if (pending != m_pendingSaveCounts.end()) {
				if (--pending->second == 0) {
					m_pendingSaveCounts.erase(pending);
				}
			}
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
			const ChunkCoord center = m_streamCoord;

			auto nearestTask = m_chunkLoadTasks.end();
			int64_t nearestDistance = std::numeric_limits<int64_t>::max();
			for (auto it = m_chunkLoadTasks.begin();
				it != m_chunkLoadTasks.end(); ++it) {
				if (m_pendingSaveCounts.contains(it->coord)) {
					continue;
				}

				const int64_t distance = std::max(
					std::abs(it->coord.x - center.x),
					std::abs(it->coord.z - center.z)
				);
				if (distance < nearestDistance) {
					nearestDistance = distance;
					nearestTask = it;
				}
			}

			if (nearestTask == m_chunkLoadTasks.end()) {
				break;
			}

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
			//保存済みチャンクを使う

			{
				std::lock_guard<std::mutex> lock(m_loadResultMutex);

				m_chunkLoadedResult.push_back(std::move(*result.data));
			}


			break;

		case ChunkLoadStatus::NotFound:
			
			break;

		case ChunkLoadStatus::Corrupted:
			//壊れているのでログ・復旧方針
			break;

		case ChunkLoadStatus::IOError:
			
			pendingReloadTasks.push_back(std::move(task));
			retryLoad = true;

			break;
		}

		if (!retryLoad) {
			std::lock_guard<std::mutex> lock(m_TaskMutex);
			m_pendingLoadKeys.erase(task.coord);
		}


	}

	if (!pendingReloadTasks.empty()) {

		std::lock_guard<std::mutex> lock(m_TaskMutex);


		for (auto& task : pendingReloadTasks) {

			
			m_chunkLoadTasks.push_back(std::move(task));
		}
	}

}


bool PersistenceIO::CheckDataExistence(ChunkCoord coord) {
	{
		std::lock_guard<std::mutex> lock(m_TaskMutex);
		if (m_pendingSaveCounts.contains(coord)) {
			return true;
		}
	}

	return c_diskStorage.CheckDataExistence(coord);

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


bool PersistenceIO::ResetWorldStorage() {
	const bool restartThread = threadRunning.load();
	if (restartThread) {
		StopThread();
	}

	{
		std::lock_guard<std::mutex> lock(m_TaskMutex);
		m_chunkSaveTasks.clear();
		m_chunkLoadTasks.clear();
		m_pendingLoadKeys.clear();
		m_pendingSaveCounts.clear();
	}

	{
		std::lock_guard<std::mutex> lock(m_loadResultMutex);
		m_chunkLoadedResult.clear();
	}

	const bool removed = w_diskStorage.DeleteWorldFromDisk();

	if (restartThread) {
		StartThread();
	}

	return removed;
}
