#include "World/WorldThread.h"



void WorldThread::StartLoop() {

	if (runningWorldThread.load()) {
		return;
	}

	runningWorldThread.store(true);

	m_chunkPipeline.StartWorkerThread();

	worldThread = std::thread([this]() {
		while (runningWorldThread.load()) {

			Tick();

		}

	});

}



void WorldThread::StopLoop() {

	runningWorldThread.store(false);


	if (worldThread.joinable()) {
		worldThread.join();
	}


	m_chunkPipeline.StopWorkerThread();
}

void WorldThread::SubmitEditBlock(
	int32_t worldX,
	int32_t worldY,
	int32_t worldZ,
	BlockType b
) {

	WorldCommand cmd;

	cmd.type = WorldCommandType::EDIT_BLOCK;
	cmd.worldX = worldX;
	cmd.worldY = worldY;
	cmd.worldZ = worldZ;

	cmd.newBlock = b;

	{
		std::lock_guard<std::mutex> lock(commandMutex);
		m_commands.push_back(cmd);
	}
}


void WorldThread::ProcCommands() {
	std::deque<WorldCommand> commands;

	{
		std::lock_guard<std::mutex> lock(commandMutex);

		commands.swap(m_commands);
	}


	for (auto& cmd : commands) {

		ApplyCommand(cmd);
	}


}


void WorldThread::ApplyCommand(WorldCommand& cmd) {

	switch (cmd.type) {
		case WorldCommandType::EDIT_BLOCK: {

			
			ApplyEditBlock(
				cmd.worldX,
				cmd.worldY,
				cmd.worldZ,
				cmd.newBlock

			);



			break;

		}

	}

}


void WorldThread::ApplyEditBlock(
	int32_t x,
	int32_t y,
	int32_t z,
	BlockType b
) {
	if (y >= Chunk::CHUNK_HEIGHT || y < 0) return;

	Chunk* c = m_world.GetTargetChunk(x, z);
	if (!c) return;

	m_world.SetBlockGlobal_User(x, y, z, b);

	Start_BlockLightTask(x, y, z, 14);

	m_world.MarkChunkUrgentDirty(*c);

}


void WorldThread::Start_BlockLightTask(
	int32_t x,
	int32_t y,
	int32_t z,
	uint8_t level

) {
	if (y >= Chunk::CHUNK_HEIGHT || y < 0) return;

	int64_t wx = static_cast<int64_t>(x) * Chunk::CHUNK_WIDTH;
	int64_t wy = y;
	int64_t wz = static_cast<int64_t>(z) * Chunk::CHUNK_DEPTH;


	LightTask task;
	task.lightType = LightType::BLOCK;


	m_lightEngine.AddLightLevel(
		m_world,
		wx,
		wy,
		wz,
		level,
		task
	);

	m_lightTasks.push_back(task);

}


void WorldThread::Tick() {

	ApplyStreamCenter();

	if (m_streamNeedsUpdate) {
		UpdateChunksAround();
	}

	ProcCommands();
	ProcChunkResults();
}


void WorldThread::UpdateChunksAround() {

	int createBudget = MAX_CHUNK_CREATE_PER_TICK;
	int destroyBudget = MAX_CHUNK_DESTROY_PER_TICK;

	bool createDone = false;

	bool allRequestedChunks_Created = true;
	bool allRequestedChunks_Destroyed = true;

	auto& chunks = m_world.GetChunks();

	for (int32_t r = 0; r <= LOAD_CHUNKS_DISTANCE && !createDone; ++r) {//Ç±ÇÍì™Ç¢Ç¢é©ï™ÇÃé¸àÕÇ©ÇÁloadÇ∑ÇÈÉAÉãÉSÉäÉYÉÄÇæÇÌ
		for (int32_t dx = -r; dx <= r && !createDone; ++dx) {
			for (int32_t dz = -r; dz <= r; ++dz) {
				if (std::max(std::llabs(dx), std::llabs(dz)) != r) {//ì‡ë§ÇÕèàóùçœÇ›Ç»ÇÃÇ≈äOé¸ÇæÇØ
					continue;
				}

				if (createBudget <= 0) {
					createDone = true;
					break;
				}

				int32_t cx = m_lastStreamCx + dx;
				int32_t cz = m_lastStreamCz + dz;

				uint64_t key = Index(cx, cz);

				if (!chunks.contains(key)) {

					allRequestedChunks_Created = false;
				}

				if (chunks.find(key) != chunks.end()) {
					continue;
				}

				if (m_pendingChunkKeys.find(key) != m_pendingChunkKeys.end()) {
					continue;
				}

				if (createBudget <= 0) {
					createDone = true;
					break;
				}

				ChunkJob job;
				job.cx = cx;
				job.cz = cz;
				job.type = JobType::CREATE_CHUNK;


				m_pendingChunkKeys.insert(key);
				m_chunkPipeline.EnqueueJob(std::move(job));


				createBudget--;

			}
		}
	}

	/*for (int64_t cx = curCx - LOAD_CHUNKS_DISTANCE; cx <= curCx + LOAD_CHUNKS_DISTANCE && !createDone; ++cx) {
		for (int64_t cz = curCz - LOAD_CHUNKS_DISTANCE; cz <= curCz + LOAD_CHUNKS_DISTANCE; ++cz) {
			uint64_t key = Index(cx, cz);

			if (createBudget <= 0) {
				createDone = true;
				break;
			}

			if (chunks.find(key) != chunks.end()) {
				continue;
			}

			std::unique_ptr<Chunk> c = std::make_unique<Chunk>(cx, cz);

			TerrainGenerator::GenerateTerrain(*c);
			chunks[key] = std::move(c);

			createBudget--;
		}
	}*/


	for (auto it = chunks.begin(); it != chunks.end();) {
		const auto& c = it->second;

		if (destroyBudget <= 0) {
			break;
		}

		bool shouldDestroy = false;
		if (!c) {
			shouldDestroy = true;
		}
		else {

			int32_t dx = c->cx - m_lastStreamCx;
			int32_t dz = c->cz - m_lastStreamCz;


			if (std::abs(dx) >= UNLOAD_CHUNKS_DISTANCE ||
				std::abs(dz) >= UNLOAD_CHUNKS_DISTANCE) {

				allRequestedChunks_Destroyed = false;

				shouldDestroy = true;
			}
		}


		if (shouldDestroy) {
			it = chunks.erase(it);
			destroyBudget--;
		}
		else {
			++it;//è¡ÇµÇΩÇ∆Ç´ÇÕóvëfÇ™é©ìÆÇ≈ãlÇﬂÇÁÇÍÇÈÇ©ÇÁè¡ÇµÇƒÇ»Ç¢Ç∆Ç´ÇæÇØitÇëùÇ‚ÇµÇƒéüÇÃóvëf
		}

	}



	if (allRequestedChunks_Created && allRequestedChunks_Destroyed) {
		m_streamNeedsUpdate = false;//Ç‡Ç§chunkà⁄ìÆÇ…ÇÊÇ¡Çƒê∂Ç∂ÇÈload, unloadÇ™ëSïîèàóùçœÇ›
	}

}



void WorldThread::SetDesiredStreamCenter(
	int32_t cx,
	int32_t cz
) {

	std::lock_guard<std::mutex> lock(streamCenterMutex);

	m_streamCx = cx;
	m_streamCz = cz;

}


void WorldThread::ApplyStreamCenter() {

	int32_t curCenterCx = 0;
	int32_t curCenterCz = 0;

	{
		std::lock_guard<std::mutex> lock(streamCenterMutex);

		curCenterCx = m_streamCx;
		curCenterCz = m_streamCz;
	}


	bool enternedNewChunk =
		m_lastStreamCx != curCenterCx ||
		m_lastStreamCz != curCenterCz;

	m_lastStreamCx = curCenterCx;
	m_lastStreamCz = curCenterCz;

	if (enternedNewChunk) {
		m_chunkPipeline.SetStreamCenter(curCenterCx, curCenterCz);

		std::vector<uint64_t> canceledKey =
			m_chunkPipeline.CancelQueuedOutside_ChunkJob();

		for (auto& key : canceledKey) {
			m_pendingChunkKeys.erase(key);
		}

		m_streamNeedsUpdate = true;

	}

}


void WorldThread::ProcChunkResults() {
	MeshChunkResult meshResult;
	GeneratedChunkResult genResult;

	auto& chunks = m_world.GetChunks();

	while (m_chunkPipeline.PopFrontMeshResult(meshResult)) {

		const auto& key = meshResult.key;

		int32_t cx = 0;
		int32_t cz = 0;


		if (meshResult.meshData) {
			PendingMesh mesh;
			mesh.meshData = std::move(*meshResult.meshData);
			mesh.key = key;

			PushPendingMesh(mesh);
		}

		if (meshResult.wasNewChunk) {
			m_world.MarkNeighborChunksDirty(cx, cz);
		}

	}


	while (m_chunkPipeline.PopFrontGenResult(genResult)) {

		const auto& key = genResult.key;

		int32_t cx = 0;
		int32_t cz = 0;


		m_pendingChunkKeys.erase(key);



		if (!genResult.chunk) {

			assert(false && "GenerateChunkResult doesnt have Chunk pointer");

		}

		chunks[key] = std::move(genResult.chunk);

		Start_SkyLightTaskForNewChunk(*chunks[key].get());

	}
}


void WorldThread::EnqueueMeshJob(Chunk& c) {

	ChunkJob job;
	job.cx = c.cx;
	job.cz = c.cz;
	job.snapshot = m_world.CreateMeshSnapshot(c);
	job.type = JobType::BUILD_MESH;

	job.isNewChunk = false;

	if (c.urgentUpdateMesh) {
		c.urgentUpdateMesh = false;
		job.urgent = true;
	}


	m_chunkPipeline.EnqueueJob(std::move(job));

}


bool WorldThread::PopPendingMeshData(PendingMesh& out) {

	std::lock_guard<std::mutex> lock(pendingMeshMutex);
	
	if (m_pendingMeshData.empty()) {
		return false;
	}

	out = std::move(m_pendingMeshData.front());
	m_pendingMeshData.pop_front();
	return true;
}


void WorldThread::PushPendingMesh(PendingMesh& mesh) {
	
	std::lock_guard<std::mutex> lock(pendingMeshMutex);

	m_pendingMeshData.push_back(mesh);

}



void WorldThread::Start_SkyLightTaskForNewChunk(Chunk& c) {

	LightTask task;
	task.lightType = LightType::SKY;

	int64_t wx = static_cast<int64_t>(c.cx) * Chunk::CHUNK_WIDTH;
	int64_t wz = static_cast<int64_t>(c.cz) * Chunk::CHUNK_DEPTH;


	for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x) {
			for (int z = 0; z < Chunk::CHUNK_DEPTH; ++z) {

				if (c.GetSkyLight(x, y, z) == 15) {
					task.bfs_queue.push({ wx + x, y, wz + z, 15 });
				}

			}
		}
	}


	m_lightTasks.push_back(task);
}



void WorldThread::ProcLightTasks() {

	int budget = MAX_LIGHT_PROPAGATE_BFS_PER_TICK;

	const size_t taskCount = m_lightTasks.size();
	const int weightSum = 2 + (taskCount - 1);
	const int normalBudget = budget / weightSum;
	int frontBudget = normalBudget * 2;

	int remainder = budget - normalBudget * (taskCount + 1);
	frontBudget += remainder;

	size_t i = 0;
	while (i < m_lightTasks.size() &&
		   budget > 0 &&
		   !m_lightTasks.empty()) {
		
		int usedBudget = normalBudget;
		auto& task = m_lightTasks[i];
		
		if (i == 0) {
			usedBudget = frontBudget;
		}

		if (task.lightType == LightType::SKY) {
			m_lightEngine.Propagate_SkyLight(
				m_world,
				task,
				usedBudget
			);
		}


		if (task.bfs_queue.empty()) {
			for (auto& key : task.touchedChunkKeys) {

				Chunk* c = m_world.GetTargetChunkFromKey(key);
				c->dirty = true;

				if (task.lightType == LightType::BLOCK) {
					c->urgentUpdateMesh = true;
				}

			}

			m_lightTasks.erase(m_lightTasks.begin() + i);
			continue;//eraseÇµÇƒÇÈÇÃÇ≈ãlÇﬂÇÈÇÃÇ≈++iîÚÇŒÇ∑
		}


		++i;

	}



}