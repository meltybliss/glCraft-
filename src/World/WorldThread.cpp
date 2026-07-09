#include "World/WorldThread.h"
#include <iostream>



void WorldThread::StartLoop() {

	if (runningWorldThread.load()) {
		return;
	}

	runningWorldThread.store(true);

	m_chunkPipeline.SetResultReadyCallback(
		[this]() {
			Wake();
		}

	);
	m_chunkPipeline.StartWorkerThread();


	BuildLoadOffsets();


	worldThread = std::thread([this]() {


		using clock = std::chrono::steady_clock;

		constexpr float FIXED_DT = 1.0f / 60.0f;
		constexpr auto SIM_INTERVAL = std::chrono::duration<float>(FIXED_DT);

		auto nextSimTime = clock::now();

		while (runningWorldThread.load()) {

			auto now = clock::now();

			if (now >= nextSimTime) {
				TickSimulation(FIXED_DT);

				nextSimTime += std::chrono::duration_cast<clock::duration>(SIM_INTERVAL);
			}

			TickBackground(nextSimTime);


			if (HasImmediateTask()) {
				continue;//skip "wait"
			}

			{
				std::unique_lock<std::mutex> lock(waitMutex);

				worldCv.wait_until(lock, nextSimTime, [this]() {

					return !runningWorldThread.load() || requestedToWake;
				});
			}


			requestedToWake = false;

		}

	});

}



void WorldThread::StopLoop() {

	runningWorldThread.store(false);


	Wake();

	if (worldThread.joinable()) {
		worldThread.join();
	}


	m_chunkPipeline.StopWorkerThread();
}

void WorldThread::SubmitEditBlock(
	int64_t worldX,
	int64_t worldY,
	int64_t worldZ,
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


	Wake();

}



void WorldThread::ProcOneCommand() {

	WorldCommand cmd;

	{
		std::lock_guard<std::mutex> lock(commandMutex);

		if (m_commands.empty()) {
			return;
		}

		cmd = std::move(m_commands.front());
		m_commands.pop_front();


	}
	ApplyCommand(cmd);

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
	int64_t x,
	int64_t y,
	int64_t z,
	BlockType b
) {
	if (y >= Chunk::CHUNK_HEIGHT || y < 0) return;

	const int32_t cx = floorDiv(x, Chunk::CHUNK_WIDTH);
	const int32_t cz = floorDiv(z, Chunk::CHUNK_DEPTH);

	Chunk* c = m_world.GetTargetChunk(cx, cz);
	if (!c) return;

	const BlockType oldBlock = static_cast<BlockType>(m_world.GetBlockGlobal(x, y, z));
	const uint8_t oldLight = m_world.GetBlockLightGlobal(x, y, z);

	const bool oldIsAir = (oldBlock == BlockType::AIR);
	const bool newIsAir = (b == BlockType::AIR);

	m_world.SetBlockGlobal_User(x, y, z, b);


	if (oldIsAir && !newIsAir) {


		Start_RemoveBlockLightTask_WithEmissionTask(
			x,
			y,
			z,
			GetEmission(b),
			true
		);

		Start_RemoveSkyLightTask(
			x,
			y,
			z,
			true
		);

	}
	else if (!oldIsAir && newIsAir) {

		if (isLightSourceBlock(oldBlock)) {
			Start_RemoveBlockLightTask(
				x,
				y,
				z,
				true
			);
		}
		else {

			Start_BlockLightTaskFromNeighbors(
				x,
				y,
				z,
				true
			);
		}


		Add_SkylightTask(x, y, z, true);

		
	}


	m_world.MarkChunkUrgentDirty(*c);

}


void WorldThread::Start_BlockLightTaskFromNeighbors(
	int64_t x,
	int64_t y,
	int64_t z,
	bool urgent
) {

	static constexpr int dirs[6][3] = {
		{ 1, 0, 0 },
		{-1, 0, 0 },
		{ 0, 1, 0 },
		{ 0,-1, 0 },
		{ 0, 0, 1 },
		{ 0, 0,-1 }
	};

	uint8_t strongest = 0;

	for (const auto& dir : dirs) {
		int64_t nx = x + dir[0];
		int64_t ny = y + dir[1];
		int64_t nz = z + dir[2];

		if (ny < 0 || ny >= Chunk::CHUNK_HEIGHT) {
			continue;
		}

		uint8_t light = m_world.GetBlockLightGlobal(nx, ny, nz);

		if (light <= 0) continue;

		if (strongest < light) {
			strongest = light;
		}
	}

	if (strongest <= 0) return;

	Start_BlockLightTask(
		x,
		y,
		z,
		strongest - 1,
		urgent
	);

}


void WorldThread::Start_RemoveBlockLightTask(
	int64_t x,
	int64_t y,
	int64_t z,
	bool urgent
) {
	if (y >= Chunk::CHUNK_HEIGHT || y < 0) return;
	

	LightTask task;

	task.lightType = LightType::BLOCK;
	task.phase = Phase::REMOVE;
	task.emissionAfterRemove = 0;

	m_lightEngine.StartRemoveBlockLightTask(
		m_world,
		x,
		y,
		z,
		task
	);

	if (task.remove_queue.empty()) return;

	if (!urgent) {
		
		m_lightTasks.push_back(std::move(task));
		
	}
	else {
		
		m_urgentLightTasks.push_back(std::move(task));
		
	}

}


void WorldThread::Start_RemoveBlockLightTask_WithEmissionTask(
	int64_t x,
	int64_t y,
	int64_t z,
	uint8_t emissionAfterRemove,
	bool urgent
) {
	if (y >= Chunk::CHUNK_HEIGHT || y < 0) return;


	LightTask task;


	task.lightType = LightType::BLOCK;
	task.phase = Phase::REMOVE;
	task.emissionAfterRemove = emissionAfterRemove;

	m_lightEngine.StartRemoveBlockLightTask(
		m_world,
		x,
		y,
		z,
		task
	);

	if (!task.remove_queue.empty() || !task.bfs_queue.empty()) {
		if (urgent) {
			m_urgentLightTasks.push_back(std::move(task));
		}
		else {
			m_lightTasks.push_back(std::move(task));
		}
	}

}


void WorldThread::Start_RemoveSkyLightTask(
	int64_t x,
	int64_t y,
	int64_t z,
	bool urgent

) {

	if (y >= Chunk::CHUNK_HEIGHT || y < 0) return;

	LightTask task;
	task.lightType = LightType::SKY;
	task.phase = Phase::REMOVE;

	m_lightEngine.StartRemoveSkyLightTask(
		m_world,
		x,
		y,
		z,
		task
	);

	if (task.remove_queue.empty()) return;
	if (!urgent) {
		m_lightTasks.push_back(std::move(task));
	}
	else {
		m_urgentLightTasks.push_back(std::move(task));
	}

}

void WorldThread::Start_BlockLightTask(
	int64_t x,
	int64_t y,
	int64_t z,
	uint8_t level,
	bool urgent

) {
	if (y >= Chunk::CHUNK_HEIGHT || y < 0) return;

	const int32_t cx = floorDiv(x, Chunk::CHUNK_WIDTH);
	const int32_t cz = floorDiv(z, Chunk::CHUNK_DEPTH);

	uint64_t key = Index(cx, cz);
	Chunk* c = m_world.GetTargetChunkFromKey(key);

	if (!c) return;

	c->readyForMesh = false;


	LightTask task;
	task.lightType = LightType::BLOCK;


	m_lightEngine.AddLightLevel(
		m_world,
		x,
		y,
		z,
		level,
		task
	);

	if (urgent) {
		m_urgentLightTasks.push_back(std::move(task));
	}
	else {
		m_lightTasks.push_back(std::move(task));
	}

}


void WorldThread::Start_SkyLightTask(
	int64_t x,
	int64_t y,
	int64_t z,
	uint8_t level,
	bool urgent
) {
	if (y < 0 || y >= Chunk::CHUNK_HEIGHT) return;
	if (level == 0) return;

	LightTask task;
	task.lightType = LightType::SKY;

	m_lightEngine.AddSkyLightLevel(
		m_world,
		x,
		y,
		z,
		level,
		task
	);

	if (task.bfs_queue.empty()) return;
	
	if (urgent) {
		m_urgentLightTasks.push_back(std::move(task));
	}
	else {
		m_lightTasks.push_back(std::move(task));
	}
}


void WorldThread::Add_SkylightTask(
	int64_t x,
	int64_t y,
	int64_t z,
	bool urgent
) {

	if (y < 0 || y >= Chunk::CHUNK_HEIGHT) {
		return;
	}

	static constexpr int dirs[6][3] = {
		{ 1, 0, 0 },
		{-1, 0, 0 },
		{ 0, 1, 0 },
		{ 0,-1, 0 },
		{ 0, 0, 1 },
		{ 0, 0,-1 }
	};

	bool directSky = (y == Chunk::CHUNK_HEIGHT - 1);

	if (!directSky) {
		const bool aboveIsAir =
			m_world.GetBlockGlobal(x, y + 1, z) == 0;

		const uint8_t aboveSky =
			m_world.GetSkyLightGlobal(x, y + 1, z);

		if (aboveIsAir && aboveSky == 15) {
			
			Start_SkyLightTask(x, y, z, 15, urgent);
			return;
		}

	}

	uint8_t strongest = 0;

	for (const auto& dir : dirs) {
		const int64_t nx = x + dir[0];
		const int64_t ny = y + dir[1];
		const int64_t nz = z + dir[2];

		if (ny < 0 || ny >= Chunk::CHUNK_HEIGHT) {
			continue;
		}

		strongest = std::max(
			strongest,
			m_world.GetSkyLightGlobal(nx, ny, nz)
		);
	}

	if (strongest > 1) {
		Start_SkyLightTask(x, y, z, strongest - 1, urgent);
	}
}




void WorldThread::TickSimulation(float dt) {

	ApplyStreamCenter();


	if (m_hasMovedMouse.load()) {
		ApplyMouseMovement();
	}

	ApplyPlayerStatus(dt);


	m_plr.Tick(dt, m_world);

	UpdatePlrSnapshot();


}


void WorldThread::TickBackground(std::chrono::steady_clock::time_point deadline) {

	using clock = std::chrono::steady_clock;

	while (clock::now() < deadline) {
		if (m_streamNeedsUpdate) {
			UpdateChunksAround_step();
		}
		if (clock::now() >= deadline) break;

		ProcOneCommand();
		if (clock::now() >= deadline) break;

		ProcOneChunkResult();
		if (clock::now() >= deadline) break;

		ProcLightTasks();
		if (clock::now() >= deadline) break;

		DispatchOneDirtyMeshJob();
		if (clock::now() >= deadline) break;
	}

}



void WorldThread::BuildLoadOffsets() {

	m_loadOffsets.clear();
	m_nextLoadOffset = 0;

	for (int32_t r = 0; r <= LOAD_CHUNKS_DISTANCE; ++r) {//Ç±ÇÍÇ¢Ç¢é©ï™ÇÃé¸àÕÇ©ÇÁloadÇ∑ÇÈÉAÉãÉSÉäÉYÉÄ
		for (int32_t dx = -r; dx <= r; ++dx) {
			for (int32_t dz = -r; dz <= r; ++dz) {
				if (std::max(std::llabs(dx), std::llabs(dz)) != r) {//ì‡ë§ÇÕèàóùçœÇ›Ç»ÇÃÇ≈äOé¸ÇæÇØ
					continue;
				}


				m_loadOffsets.push_back({ dx, dz });
			}
		}
	}

}



bool WorldThread::RequestOneMissingChunkAround() {


	auto& chunks = m_world.GetChunks();

	while (m_nextLoadOffset < m_loadOffsets.size()) {

		auto& offset = m_loadOffsets[m_nextLoadOffset];
		auto& dx = offset.dx;
		auto& dz = offset.dz;



		int32_t cx = m_lastStreamCx + dx;
		int32_t cz = m_lastStreamCz + dz;

		m_nextLoadOffset++;


		uint64_t key = Index(cx, cz);


		if (chunks.find(key) != chunks.end()) {
			continue;
		}


		if (m_pendingChunkKeys.find(key) != m_pendingChunkKeys.end()) {

			continue;

		}

		ChunkJob job;
		job.cx = cx;
		job.cz = cz;
		job.type = JobType::CREATE_CHUNK;


		m_pendingChunkKeys.insert(key);
		m_chunkPipeline.EnqueueJob(std::move(job));

		return true;
	}

	return false;
}


bool WorldThread::HasChunkToErase() {
	auto& chunks = m_world.GetChunks();

	for (auto& [key, c] : chunks) {

		if (!c) return true;

		int32_t dx = c->cx - m_lastStreamCx;
		int32_t dz = c->cz - m_lastStreamCz;

		if (std::abs(dx) >= UNLOAD_CHUNKS_DISTANCE ||
			std::abs(dz) >= UNLOAD_CHUNKS_DISTANCE) {
			return true;
		}
	}

	return false;
}



bool WorldThread::HasChunkToCreate() {

	auto& chunks = m_world.GetChunks();

	for (const auto& offset : m_loadOffsets) {

		int32_t cx = m_lastStreamCx + offset.dx;
		int32_t cz = m_lastStreamCz + offset.dz;

		uint64_t key = Index(cx, cz);

		bool alreadyLoaded =
			chunks.find(key) != chunks.end();

		bool alreadyPending =
			m_pendingChunkKeys.find(key) != m_pendingChunkKeys.end();

		if (!alreadyLoaded && !alreadyPending) {
			return true;
		}

	}


	return false;
}


bool WorldThread::RequestEraseOneChunkAround() {
	auto& chunks = m_world.GetChunks();

	for (auto it = chunks.begin(); it != chunks.end(); ++it) {
		const auto& c = it->second;

		bool shouldDestroy = false;

		if (!c) {
			shouldDestroy = true;
		}
		else {
			int32_t dx = c->cx - m_lastStreamCx;
			int32_t dz = c->cz - m_lastStreamCz;

			if (std::abs(dx) >= UNLOAD_CHUNKS_DISTANCE ||
				std::abs(dz) >= UNLOAD_CHUNKS_DISTANCE) {
				shouldDestroy = true;
			}
		}

		if (!shouldDestroy) {
			continue;
		}

		{
			std::lock_guard<std::mutex> lock(pendingDeleteMeshMutex);
			m_pendingDeleteMeshKey.push_back(it->first);
		}

		chunks.erase(it);

		return true; 
	}

	return false; 
}

void WorldThread::UpdateChunksAround_step() {


	bool hasChunksToCreate = true;
	bool hasChunksToErase = true;

	RequestOneMissingChunkAround();

	hasChunksToCreate = HasChunkToCreate();

	RequestEraseOneChunkAround();

	hasChunksToErase = HasChunkToErase();

	if (!hasChunksToCreate) {
		m_nextLoadOffset = 0;
	}

	if (!hasChunksToCreate && !hasChunksToErase) {
		m_streamNeedsUpdate = false;//Ç‡Ç§chunkà⁄ìÆÇ…ÇÊÇ¡Çƒê∂Ç∂ÇÈload, unloadÇ™ëSïîèàóùçœÇ›
	}
}


void WorldThread::UpdateChunksAround() {

	int createBudget = MAX_CHUNK_CREATE_PER_TICK;
	int destroyBudget = MAX_CHUNK_DESTROY_PER_TICK;

	bool createDone = false;

	bool allRequestedChunks_Created = true;
	bool allRequestedChunks_Destroyed = true;

	auto& chunks = m_world.GetChunks();

	for (int32_t r = 0; r <= LOAD_CHUNKS_DISTANCE && !createDone; ++r) {//Ç±ÇÍÇ¢Ç¢é©ï™ÇÃé¸àÕÇ©ÇÁloadÇ∑ÇÈÉAÉãÉSÉäÉYÉÄ
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


			std::lock_guard<std::mutex> lock(pendingDeleteMeshMutex);

			m_pendingDeleteMeshKey.push_back(it->first);

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

	{
		std::lock_guard<std::mutex> lock(streamCenterMutex);

		m_streamCx = cx;
		m_streamCz = cz;
	}

	m_hasSettedDesireStreamC.store(true);

	Wake();
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


	m_hasSettedDesireStreamC.store(false);

}


void WorldThread::ProcOneChunkResult() {
	MeshChunkResult meshResult;
	GeneratedChunkResult genResult;

	auto& chunks = m_world.GetChunks();

	if (m_chunkPipeline.PopFrontMeshResult(meshResult)) {
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

	if (m_chunkPipeline.PopFrontGenResult(genResult)) {

		const auto& key = genResult.key;

		int32_t cx = 0;
		int32_t cz = 0;


		m_pendingChunkKeys.erase(key);



		if (!genResult.chunk) {

			assert(false && "GenerateChunkResult doesnt have Chunk pointer");

		}


		chunks[key] = std::move(genResult.chunk);


		Start_SkyLightTaskForNewChunk(*chunks[key]);

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


		Start_SkyLightTaskForNewChunk(*chunks[key]);
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


bool WorldThread::PopPendingDeleteMeshKey(uint64_t& out) {

	std::lock_guard<std::mutex> lock(pendingDeleteMeshMutex);

	if (m_pendingDeleteMeshKey.empty()) {
		return false;
	}

	out = std::move(m_pendingDeleteMeshKey.front());
	m_pendingDeleteMeshKey.pop_front();

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


	m_lightEngine.InitializeSkylightForChunk(c);


	m_lightEngine.CreateSkylightLeakSeeds(c, task);

	
	if (task.bfs_queue.empty()) {
		c.dirty = true;
		c.readyForMesh = true;
		return;
	}
	

	c.readyForMesh = false;

	
	m_lightTasks.push_back(task);
	
}



void WorldThread::ProcessLightTask(LightTask& task, int& budget) {
	if (task.lightType == LightType::SKY) {
		if (task.phase == Phase::ADD) {
			m_lightEngine.Propagate_SkyLight(
				m_world,
				task,
				budget
			);
		}
		else if (task.phase == Phase::REMOVE) {
			bool ok = m_lightEngine.Propagate_RemoveSkylight(
				m_world,
				task,
				budget
			);

			if (ok) {
				m_lightEngine.Propagate_SkyLight(
					m_world,
					task,
					budget
				);
			}
		}

	}
	else if (task.lightType == LightType::BLOCK) {

		if (task.phase == Phase::ADD) {
			m_lightEngine.Propagate_BlockLight(
				m_world,
				task,
				budget
			);
		}
		else if (task.phase == Phase::REMOVE) {
			bool ok = m_lightEngine.Propagate_RemoveBlockLight(
				m_world,
				task,
				budget
			);

			if (ok) {

				m_lightEngine.Propagate_BlockLight(
					m_world,
					task,
					budget
				);

			}
		}
	}


}



void WorldThread::FinishLightTask(LightTask& task) {
	for (auto& key : task.touchedChunkKeys) {

		Chunk* c = m_world.GetTargetChunkFromKey(key);
		c->dirty = true;
		c->readyForMesh = true;

		if (task.lightType == LightType::BLOCK) {
			c->urgentUpdateMesh = true;
		}

		m_world.MarkNeighborChunksUrgentDirty(c->cx, c->cz);

	}


}


void WorldThread::ProcLightTasks() {

	if (m_lightTasks.empty() && m_urgentLightTasks.empty()) return;

	int budget = MAX_LIGHT_PROPAGATE_BFS_PER_TICK;

	const size_t taskCount = m_lightTasks.size() + m_urgentLightTasks.size();
	const int weightSum = 2 + (taskCount - 1);
	const int normalBudget = budget / weightSum;
	int frontBudget = normalBudget * 2;

	int remainder = budget - normalBudget * (taskCount + 1);
	frontBudget += remainder;

	bool frontBudgetUsed = false;

	{
		size_t i = 0;

		while (i < m_urgentLightTasks.size() &&
			budget > 0 &&
			!m_urgentLightTasks.empty()) {

			int usedBudget = normalBudget;
			auto& task = m_urgentLightTasks[i];

			if (i == 0) {
				if (!frontBudgetUsed) {
					usedBudget = frontBudget;
					frontBudgetUsed = true;
				}
			}

			ProcessLightTask(task, usedBudget);


			bool finished = false;

			if (task.phase == Phase::ADD) {
				finished = task.bfs_queue.empty();
			}
			else if (task.phase == Phase::REMOVE) {
				finished = task.remove_queue.empty();
			}

			if (finished) {
				
				FinishLightTask(task);

				m_urgentLightTasks.erase(m_urgentLightTasks.begin() + i);
				continue;//eraseÇµÇƒÇÈÇÃÇ≈ãlÇﬂÇÈÇÃÇ≈++iîÚÇŒÇ∑
			}


			++i;

		}

	}

	size_t i = 0;
	while (i < m_lightTasks.size() &&
		   budget > 0 &&
		   !m_lightTasks.empty()) {
		
		int usedBudget = normalBudget;
		auto& task = m_lightTasks[i];

		
		if (i == 0) {
			if (!frontBudgetUsed) {
				usedBudget = frontBudget;
				frontBudgetUsed = true;
			}
		}


		ProcessLightTask(task, usedBudget);



		bool finished = false;

		if (task.phase == Phase::ADD) {
			finished = task.bfs_queue.empty();
		}
		else if (task.phase == Phase::REMOVE) {
			finished = task.remove_queue.empty();
		}

		if (finished) {
			
			FinishLightTask(task);

			m_lightTasks.erase(m_lightTasks.begin() + i);
			continue;//eraseÇµÇƒÇÈÇÃÇ≈ãlÇﬂÇÈÇÃÇ≈++iîÚÇŒÇ∑
		}


		++i;

	}



}



void WorldThread::DispatchDirtyMeshJobs() {

	auto& chunks = m_world.GetChunks();

	for (auto& [key, chunkPtr] : chunks) {
		if (!chunkPtr->readyForMesh) continue;

		if (!chunkPtr->dirty) continue;

		//if (chunkPtr->meshJobInFlight) continue;

		EnqueueMeshJob(*chunkPtr);

		chunkPtr->dirty = false;
		chunkPtr->readyForMesh = false;
	}

}


void WorldThread::DispatchOneDirtyMeshJob() {//TODO: édëgÇ›Çunordered_setÇégÇ§Ç‚ÇËï˚Ç…ïœÇ¶ÇÈå„Ç≈

	auto& chunks = m_world.GetChunks();

	for (auto& [key, chunkPtr] : chunks) {
		if (!chunkPtr->readyForMesh) continue;

		if (!chunkPtr->dirty) continue;

		//if (chunkPtr->meshJobInFlight) continue;

		EnqueueMeshJob(*chunkPtr);

		chunkPtr->dirty = false;
		chunkPtr->readyForMesh = false;

		return;
	}

}


void WorldThread::Wake() {

	{
		std::lock_guard<std::mutex> lock(waitMutex);
		requestedToWake = true;
	}
	worldCv.notify_all();
}


bool WorldThread::HasImmediateTask() {

	return !m_lightTasks.empty() || !m_urgentLightTasks.empty() || 
		    m_streamNeedsUpdate  ||  m_hasSettedDesireStreamC.load() ||
		    m_hasSettedInput.load() || m_hasMovedMouse.load();

}



RaycastHit WorldThread::RequestRaycast(const glm::vec3& origin, const glm::vec3& dir, float distance) const {

	return m_world.Raycast(
		origin,
		dir,
		distance
	);

}


void WorldThread::ApplyPlayerStatus(float dt) {

	glm::vec3 pos = m_plr.GetPos();
	PlayerInput input;

	{
		std::lock_guard<std::mutex> lock(inputMutex);

		input = m_inputBuffer;

	}

	glm::vec3 front = m_plr.GetFront();
	glm::vec3 right = m_plr.GetRight();

	front.y = 0.0f;
	right.y = 0.0f;

	if (glm::length(front) > 0.0f) {
		front = glm::normalize(front);
	}

	if (glm::length(right) > 0.0f) {
		right = glm::normalize(right);
	}

	glm::vec3 moveDir{ 0.0f };

	if (input.forward) moveDir += front;
	if (input.back)    moveDir -= front;
	if (input.left)    moveDir -= right;
	if (input.right)   moveDir += right;

	if (glm::length(moveDir) > 0.0f) {
		moveDir = glm::normalize(moveDir);
	}

	float speed = m_plr.GetSpeed();

	m_plr.SetVelX(moveDir.x * speed);
	m_plr.SetVelZ(moveDir.z * speed);


	m_hasSettedInput.store(false);
}



void WorldThread::AddMouseDelta(float xoffset, float yoffset) {


	{
		std::lock_guard<std::mutex> lock(offsetMutex);

		m_xoffsetBuffer += xoffset;
		m_yoffsetBuffer += yoffset;
		
	}

	m_hasMovedMouse.store(true);

	Wake();


}



void WorldThread::ApplyMouseMovement() {

	float yaw = m_plr.GetYaw();
	float pitch = m_plr.GetPitch();

	float xoffset = 0.f;
	float yoffset = 0.f;

	{
		std::lock_guard<std::mutex> lock(offsetMutex);

		xoffset = m_xoffsetBuffer;
		yoffset = m_yoffsetBuffer;

		m_xoffsetBuffer = 0.f;
		m_yoffsetBuffer = 0.f;
	}

	yaw += xoffset;
	pitch += yoffset;

	m_plr.SetYaw(yaw);
	m_plr.SetPitch(pitch);

	if (m_plr.GetPitch() > 89.0f) {
		m_plr.SetPitch(89.0f);
	}

	if (m_plr.GetPitch() < -89.0f) {
		m_plr.SetPitch(-89.0f);
	}


	m_hasMovedMouse.store(false);
	m_plr.UpdateVectors();

}




void WorldThread::UpdatePlrSnapshot() {

	PlayerSnapshot snap;

	snap.front = m_plr.GetFront();
	snap.pos = m_plr.GetEyePos();
	snap.right = m_plr.GetRight();
	snap.up = m_plr.GetUp();

	{
		std::lock_guard<std::mutex> lock(snapshotMutex);

		m_plrSnapshot = std::move(snap);
	}

}