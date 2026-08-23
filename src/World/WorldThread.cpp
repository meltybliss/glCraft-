#include "World/WorldThread.h"
#include "Persistence/PersistenceIO.h"
#include "Render/Camera.h"
#include "World/TerrainGenerator.h"
#include <iostream>



void WorldThread::CreateNewWorld(uint64_t seed) {

	m_world.SetWorldSeed(seed);

	m_chunkPipeline.SetWorldSeed(seed);
}

void WorldThread::ApplyLoadedWorld(WorldSaveData& saveData) {

	m_world.SetWorldSeed(saveData.seed);
	m_chunkPipeline.SetWorldSeed(saveData.seed);
	m_world.SetWorldTime(saveData.worldTime);

	m_plr.SetPosition(saveData.playerPos);
	

}


void WorldThread::SetWorldSelectionResult(WorldSelectionResult& info) {

	m_world.SetWorldSelectionResult(info);
}

void WorldThread::StartThread() {

	if (runningWorldThread.load()) {
		return;
	}

	runningWorldThread.store(true);

	m_chunkPipeline.SetResultReadyCallback(
		[this]() {
			Wake();
		}

	);
	m_chunkPipeline.StartWorkerThreads();


	BuildLoadOffsets();


	worldThread = std::thread([this]() {


		using clock = std::chrono::steady_clock;

		constexpr float FIXED_DT = 1.0f / 60.0f;
		constexpr auto SIM_INTERVAL = std::chrono::duration<float>(FIXED_DT);

		auto nextSimTime = clock::now();
		auto nextDebugPublishTime = clock::now();

		while (runningWorldThread.load()) {
			const auto workStart = clock::now();

			auto now = clock::now();

			if (now >= nextSimTime) {
				TickSimulation(FIXED_DT, nextSimTime);

				nextSimTime += std::chrono::duration_cast<clock::duration>(SIM_INTERVAL);
			}

			TickBackground(nextSimTime);

			const auto workTime = clock::now() - workStart;
			const auto workTimeNs = std::chrono::duration_cast<
				std::chrono::nanoseconds
			>(workTime).count();

			m_debugBusyTimeNs.fetch_add(
				static_cast<uint64_t>(workTimeNs),
				std::memory_order_relaxed
			);
			m_debugLoopIterations.fetch_add(1, std::memory_order_relaxed);

			if (clock::now() >= nextDebugPublishTime) {
				PublishDebugCounters();
				nextDebugPublishTime =
					clock::now() + std::chrono::milliseconds(100);
			}


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



void WorldThread::StopThread() {

	runningWorldThread.store(false);

	Wake();

	if (worldThread.joinable()) {
		worldThread.join();
	}


	//ここで強制的にworldをsaveし、dirtyToSaveのchunksをtask化する。
	ForcedSave_World();
	ForcedSave_Chunks();



	m_chunkPipeline.StopWorkerThreads();
	m_persistenceIO.StopThread();
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


void WorldThread::ForcedSave_World() {

	m_persistenceIO.SaveWorld(CreateWorldSaveData());


}

void WorldThread::ForcedSave_Chunks() {

	QueueChunksToAutoSave();

}


void WorldThread::CheckAutoSave() {

	const auto now = Clock::now();

	if (now < m_nextAutoSaveTime) return;

	//kokode save
	QueueChunksToAutoSave();
	m_persistenceIO.SaveWorld(CreateWorldSaveData());

	m_nextAutoSaveTime += AUTO_SAVE_INTERVAL;
}


WorldSaveData WorldThread::CreateWorldSaveData() const {

	WorldSaveData data;

	const DayNightState& state = m_world.GetDayNightState();

	data.playerPos = m_plr.GetPos();
	data.worldTime = state.timeOfDay;
	data.seed = m_world.GetWorldSeed();

	data.generatorVersion = TerrainGenerator::GetVersion();
	

	return data;
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

	const ChunkCoord coord{floorDiv(x, Chunk::CHUNK_WIDTH), floorDiv(z, Chunk::CHUNK_DEPTH)};

	Chunk* c = m_world.GetTargetChunk(coord);
	if (!c) return;

	const BlockType oldBlock = static_cast<BlockType>(m_world.GetBlockGlobal(x, y, z));
	const uint8_t oldLight = m_world.GetBlockLightGlobal(x, y, z);

	const bool oldIsAir = (oldBlock == BlockType::AIR);
	const bool newIsAir = (b == BlockType::AIR);

	auto result = m_world.SetBlockGlobal_User(x, y, z, b);
	if (result.pointLightChanged) {
		m_pointLightDirty = true;
	}


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

		//これらでblock light taskが例えば終わったらmeshがアップデートされてさらにsky light taskおわったら再度またしてしまうので無駄があるかも
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
	int64_t strongestNX = 0;
	int64_t strongestNY = 0;
	int64_t strongestNZ = 0;

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
			strongestNX = nx;
			strongestNY = ny;
			strongestNZ = nz;
		}
	}

	if (strongest <= 0) return;

	glm::vec3 color = m_world.GetBlockLightColorGlobal(strongestNX, strongestNY, strongestNZ);;

	Start_BlockLightTask(
		x,
		y,
		z,
		strongest - 1,
		color,
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
	task.sourceCoord = Index(
		floorDiv(x, Chunk::CHUNK_WIDTH),
		floorDiv(z, Chunk::CHUNK_DEPTH)
	);

	task.lightType = LightType::BLOCK;
	task.phase = Phase::REMOVE;
	task.emissionAfterRemove = 0;

	task.urgent = urgent;

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
	task.sourceCoord = Index(
		floorDiv(x, Chunk::CHUNK_WIDTH),
		floorDiv(z, Chunk::CHUNK_DEPTH)
	);


	task.lightType = LightType::BLOCK;
	task.phase = Phase::REMOVE;
	task.emissionAfterRemove = emissionAfterRemove;
	task.urgent = urgent;


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
	task.sourceCoord = Index(
		floorDiv(x, Chunk::CHUNK_WIDTH),
		floorDiv(z, Chunk::CHUNK_DEPTH)
	);
	task.lightType = LightType::SKY;
	task.phase = Phase::REMOVE;
	task.urgent = urgent;


	m_lightEngine.StartRemoveSkyLightTask(
		m_world,
		x,
		y,
		z,
		task
	);

	if (task.remove_queue.empty()) { 
		
		FinishLightTask(task); 
		return;
	}


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
	const glm::vec3& color,
	bool urgent

) {
	if (y >= Chunk::CHUNK_HEIGHT || y < 0) return;

	const ChunkCoord coord{floorDiv(x, Chunk::CHUNK_WIDTH), floorDiv(z, Chunk::CHUNK_DEPTH)};
	Chunk* c = m_world.GetTargetChunkFromKey(coord);

	if (!c) return;


	LightTask task;
	task.sourceCoord = coord;
	task.lightType = LightType::BLOCK;

	task.urgent = urgent;


	m_lightEngine.AddLightLevel(
		m_world,
		x,
		y,
		z,
		level,
		color,
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
	int level,
	bool urgent
) {
	if (y < 0 || y >= Chunk::CHUNK_HEIGHT) return;
	//if (level == 0) return;

	LightTask task;
	task.sourceCoord = Index(
		floorDiv(x, Chunk::CHUNK_WIDTH),
		floorDiv(z, Chunk::CHUNK_DEPTH)
	);
	task.lightType = LightType::SKY;
	task.urgent = urgent;


	if (level <= 0) {
		m_lightEngine.InsertGeoDirtyChunks(
			m_world,
			x,
			y,
			z,
			task
		);

		FinishLightTask(task);
		return;
	}

	m_lightEngine.AddSkyLightLevel(
		m_world,
		x,
		y,
		z,
		(uint8_t)level,
		task
	);

	if (task.bfs_queue.empty()) {
		
		FinishLightTask(task); 
		return;
	}
	
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
	


	
	Start_SkyLightTask(x, y, z, strongest - 1, urgent);
	
	
}




void WorldThread::TickSimulation(float dt, std::chrono::steady_clock::time_point simTime) {

	ApplyStreamCenter();
	

	if (m_hasMovedMouse.load()) {
		ApplyMouseMovement();
	}

	ApplyPlayerStatus(dt);//include Player tick inside

	UpdateDayNightState(dt);
	UpdateDayNightSnap();

	UpdatePlrSnapshot(simTime);


}


void WorldThread::TickBackground(std::chrono::steady_clock::time_point deadline) {

	using clock = std::chrono::steady_clock;

	CheckAutoSave();


	if (clock::now() >= deadline) return;

	if (m_lightVolumeDirty.exchange(false)) {
		ProcCreateLightVSnap();
	}

	if (clock::now() >= deadline) return;

	if (m_pointLightDirty.exchange(false)) {

		ProcCreatePointLightsSnapshot();
	}


	if (clock::now() >= deadline) return;

	while (clock::now() < deadline) {
		if (m_streamNeedsUpdate) {
			UpdateChunksAround_step();
		}
		if (clock::now() >= deadline) break;

		ProcOneCommand();
		if (clock::now() >= deadline) break;

		ProcOneChunkResult();
		if (clock::now() >= deadline) break;

		ProcOne_Disk_ChunkLoadResult();

		if (clock::now() >= deadline) break;

		ProcLightTasks(deadline);
		if (clock::now() >= deadline) break;

		DispatchOneDirtyMeshJob();
		if (clock::now() >= deadline) break;
	}


}



void WorldThread::CheckBlockLightChangesForLightVolume(
	LightTask& task
) {
	if (!task.changedBlockLight) {
		return;
	}

	for (const auto& pos : task.changedBlockLightsPos) {

		if (IsInsideLightVolume(
			pos.x,
			pos.y,
			pos.z))
		{
			m_lightVolumeDirty.store(true);
			break;
		}
	}

	//今回確認した変更は消す
	task.changedBlockLight = false;
	task.changedBlockLightsPos.clear();
}


void WorldThread::UpdateDayNightSnap() {

	DayNightSnapshot snapshot;

	auto& state = m_world.GetDayNightState();

	constexpr float pi = 3.14159265359f;

	const float sunAngle =
		state.timeOfDay *
		2.0f *
		pi -
		pi * 0.5f;

	const float rawSunHeight = std::sin(sunAngle);

	const glm::vec3 directionToSun =
		glm::normalize(glm::vec3(
			std::cos(sunAngle),
			std::sin(sunAngle),
			0.0f
		));


	const float sunHeight = rawSunHeight;

	const float dayFactor =
		glm::smoothstep(
			-0.08f,
			0.12f,
			sunHeight
		);


	const float sunIntensity =
		glm::smoothstep(
			-0.02f,
			0.20f,
			sunHeight
		);

	const float skyStrength =
		glm::mix(
			0.04f,
			1.0f,
			dayFactor
		);

	
	snapshot.directionToSun = directionToSun;
	snapshot.sunHeight = sunHeight;


	snapshot.dayFactor = dayFactor;
	snapshot.sunIntensity = sunIntensity;
	snapshot.skyStrength = skyStrength;


	m_exchanger.PublishDaynightSnap(snapshot);
}


void WorldThread::UpdateDayNightState(float dt) {

	auto& state = m_world.GetDayNightState();

	if (state.paused) return;


	state.timeOfDay +=
		dt *
		state.timeScale / state.dayLengthSeconds;

	state.timeOfDay -= std::floor(state.timeOfDay);

}


void WorldThread::BuildLoadOffsets()
{
	m_loadOffsets.clear();
	m_loadOffsetsRank.clear();
	m_nextLoadOffset = 0;

	const int32_t r = LOAD_CHUNKS_DISTANCE;

	for (int32_t dx = -r; dx <= r; ++dx) {
		for (int32_t dz = -r; dz <= r; ++dz) {
			m_loadOffsets.push_back({ dx, dz });
		}
	}

	std::sort(
		m_loadOffsets.begin(),
		m_loadOffsets.end(),
		[](const auto& a, const auto& b)
		{
			const int64_t da =
				static_cast<int64_t>(a.dx) * a.dx +
				static_cast<int64_t>(a.dz) * a.dz;

			const int64_t db =
				static_cast<int64_t>(b.dx) * b.dx +
				static_cast<int64_t>(b.dz) * b.dz;

			return da < db;
		}
	);



	for (int i = 0; i < m_loadOffsets.size(); ++i) {
		auto& offset = m_loadOffsets[i];

		m_loadOffsetsRank[Index(offset.dx, offset.dz)] = i;
	}

}


bool WorldThread::RequestOneMissingChunkAround() {


	auto& chunks = m_world.GetChunks();

	auto worldInfo = m_world.GetWorldInfo();

	while (m_nextLoadOffset < m_loadOffsets.size()) {

		auto& offset = m_loadOffsets[m_nextLoadOffset];
		auto& dx = offset.dx;
		auto& dz = offset.dz;



		const ChunkCoord coord{
			m_lastStreamCoord.x + dx,
			m_lastStreamCoord.z + dz
		};

		m_nextLoadOffset++;


		if (chunks.find(coord) != chunks.end()) {
			continue;
		}


		if (m_pendingChunkKeys.find(coord) != m_pendingChunkKeys.end()) {

			continue;

		}


		if (worldInfo.action == WorldSelectionAction::LoadWorld && 
			m_persistenceIO.CheckDataExistence(coord)) {


			m_persistenceIO.RequestToLoadChunk(coord);

		}
		else {
			ChunkJob job;
			job.coord = coord;
			job.type = JobType::CREATE_CHUNK;


			m_pendingChunkKeys.insert(coord);
			m_chunkPipeline.EnqueueJob(std::move(job));
		}
		return true;
	}

	return false;
}



void WorldThread::ProcOne_Disk_ChunkLoadResult() {
	auto& chunks = m_world.GetChunks();

	std::optional<ChunkSaveData> saveData =
		m_persistenceIO.PopChunkLoadedResult();

	if (!saveData) return;

	const ChunkCoord coord = saveData->coord;
	if (!IsChunkWithinUnloadRange(coord)) return;

	auto it = chunks.find(coord);

	if (it == chunks.end()) {

		std::unique_ptr<Chunk> c = std::make_unique<Chunk>(coord);

		c->ReceiveBlocksVector(saveData->blocks);
		chunks[coord] = std::move(c);



		Start_SkyLightTaskForNewChunk(*chunks[coord]);

	}
}



bool WorldThread::HasChunkToErase() {
	auto& chunks = m_world.GetChunks();

	for (auto& [key, c] : chunks) {

		if (!c) return true;

		int64_t dx = c->coord.x - m_lastStreamCoord.x;
		int64_t dz = c->coord.z - m_lastStreamCoord.z;

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

		const ChunkCoord coord{
			m_lastStreamCoord.x + offset.dx,
			m_lastStreamCoord.z + offset.dz
		};

		bool alreadyLoaded =
			chunks.find(coord) != chunks.end();

		bool alreadyPending =
			m_pendingChunkKeys.find(coord) != m_pendingChunkKeys.end();

		if (!alreadyLoaded && !alreadyPending) {
			return true;
		}

	}


	return false;
}


bool WorldThread::IsChunkWithinUnloadRange(ChunkCoord coord) const {
	const int64_t dx =
		coord.x - m_lastStreamCoord.x;
	const int64_t dz =
		coord.z - m_lastStreamCoord.z;

	return std::abs(dx) < UNLOAD_CHUNKS_DISTANCE &&
		std::abs(dz) < UNLOAD_CHUNKS_DISTANCE;
}


void WorldThread::QueueMeshDeletion(ChunkCoord key) {
	{
		std::lock_guard<std::mutex> lock(pendingMeshMutex);

		std::erase_if(
			m_pendingMeshData,
			[key](const PendingMesh& mesh) {
				return mesh.key == key;
			}
		);
	}

	{
		std::lock_guard<std::mutex> lock(pendingDeleteMeshMutex);
		m_pendingDeleteMeshKey.push_back(key);
	}
}


void WorldThread::RebuildDirtyMeshQueuePriorities() {
	std::priority_queue<ChunkDirtyEntry> rebuiltQueue;

	for (const auto& [key, chunk] : m_world.GetChunks()) {
		if (!chunk || !chunk->dirty) continue;

		const int64_t dx = chunk->coord.x - m_lastStreamCoord.x;
		const int64_t dz = chunk->coord.z - m_lastStreamCoord.z;
		const auto rank = m_loadOffsetsRank.find(Index(dx, dz));

		if (rank == m_loadOffsetsRank.end()) continue;

		rebuiltQueue.push({
			chunk->urgentUpdateMesh ? -1 : rank->second,
			key
		});
	}

	m_dirtyMeshQueue.swap(rebuiltQueue);
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
			int64_t dx = c->coord.x - m_lastStreamCoord.x;
			int64_t dz = c->coord.z - m_lastStreamCoord.z;

			if (std::abs(dx) >= UNLOAD_CHUNKS_DISTANCE ||
				std::abs(dz) >= UNLOAD_CHUNKS_DISTANCE) {
				shouldDestroy = true;
			}
		}

		if (!shouldDestroy) {
			continue;
		}

		QueueMeshDeletion(it->first);

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
		m_streamNeedsUpdate = false;//もうchunk移動によって生じるload, unloadが全部処理済み
	}
}


void WorldThread::UpdateChunksAround() {

	int createBudget = MAX_CHUNK_CREATE_PER_TICK;
	int destroyBudget = MAX_CHUNK_DESTROY_PER_TICK;

	bool createDone = false;

	bool allRequestedChunks_Created = true;
	bool allRequestedChunks_Destroyed = true;

	auto& chunks = m_world.GetChunks();

	for (int32_t r = 0; r <= LOAD_CHUNKS_DISTANCE && !createDone; ++r) {//これいい自分の周囲からloadするアルゴリズム
		for (int32_t dx = -r; dx <= r && !createDone; ++dx) {
			for (int32_t dz = -r; dz <= r; ++dz) {
				if (std::max(std::llabs(dx), std::llabs(dz)) != r) {//内側は処理済みなので外周だけ
					continue;
				}

				if (createBudget <= 0) {
					createDone = true;
					break;
				}

				const ChunkCoord coord{
					m_lastStreamCoord.x + dx,
					m_lastStreamCoord.z + dz
				};

				if (!chunks.contains(coord)) {

					allRequestedChunks_Created = false;
				}

				if (chunks.find(coord) != chunks.end()) {
					continue;
				}

				if (m_pendingChunkKeys.find(coord) != m_pendingChunkKeys.end()) {
					continue;
				}

				if (createBudget <= 0) {
					createDone = true;
					break;
				}

				ChunkJob job;
				job.coord = coord;
				job.type = JobType::CREATE_CHUNK;


				m_pendingChunkKeys.insert(coord);
				m_chunkPipeline.EnqueueJob(std::move(job));


				createBudget--;

			}
		}
	}

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

			int64_t dx = c->coord.x - m_lastStreamCoord.x;
			int64_t dz = c->coord.z - m_lastStreamCoord.z;


			if (std::abs(dx) >= UNLOAD_CHUNKS_DISTANCE ||
				std::abs(dz) >= UNLOAD_CHUNKS_DISTANCE) {

				allRequestedChunks_Destroyed = false;

				shouldDestroy = true;
			}
		}


		if (shouldDestroy) {


			QueueMeshDeletion(it->first);

			it = chunks.erase(it);
			destroyBudget--;

		}
		else {
			++it;//消したときは要素が自動で詰められるから消してないときだけitを増やして次の要素
		}

	}



	if (allRequestedChunks_Created && allRequestedChunks_Destroyed) {
		m_streamNeedsUpdate = false;//もうchunk移動によって生じるload, unloadが全部処理済み
	}

}



void WorldThread::Rebuild_allChunks() {
	auto& chunks = m_world.GetChunks();


	for (auto& [key, chunkPtr] : chunks) {
		if (!chunkPtr) {
			continue;
		}

		MarkChunkUrgentDirty(*chunkPtr);
	}

	Wake();
}


void WorldThread::SetDesiredStreamCenter(ChunkCoord coord) {

	{
		std::lock_guard<std::mutex> lock(streamCenterMutex);

		m_streamCoord = coord;
	}

	m_hasSettedDesireStreamC.store(true);

	Wake();
}


void WorldThread::ApplyStreamCenter() {

	ChunkCoord currentCenter{};

	{
		std::lock_guard<std::mutex> lock(streamCenterMutex);

		currentCenter = m_streamCoord;
	}


	bool enternedNewChunk = m_lastStreamCoord != currentCenter;

	m_lastStreamCoord = currentCenter;

	if (enternedNewChunk) {

		m_nextLoadOffset = 0;

		m_chunkPipeline.SetStreamCenter(currentCenter);
		m_persistenceIO.SetStreamCenterAndCancelOutsideLoads(
			currentCenter,
			UNLOAD_CHUNKS_DISTANCE
		);

		std::vector<ChunkCoord> canceledKey =
			m_chunkPipeline.CancelQueuedOutside_ChunkJob();

		for (auto& key : canceledKey) {
			m_pendingChunkKeys.erase(key);
		}

		RebuildDirtyMeshQueuePriorities();

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

		if (meshResult.meshData) {
			PendingMesh mesh;
			mesh.meshData = std::move(*meshResult.meshData);
			mesh.key = key;

			PushPendingMesh(std::move(mesh));
		}

	}

	if (m_chunkPipeline.PopFrontGenResult(genResult)) {

		const auto& key = genResult.key;


		m_pendingChunkKeys.erase(key);



		if (!genResult.chunk) {

			assert(false && "GenerateChunkResult doesnt have Chunk pointer");
			return;
		}

		if (!IsChunkWithinUnloadRange(key) || chunks.contains(key)) {
			return;
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


		if (meshResult.meshData) {
			PendingMesh mesh;
			mesh.meshData = std::move(*meshResult.meshData);
			mesh.key = key;

			PushPendingMesh(std::move(mesh));
		}

	}


	while (m_chunkPipeline.PopFrontGenResult(genResult)) {

		const auto& key = genResult.key;

	
		m_pendingChunkKeys.erase(key);



		if (!genResult.chunk) {

			assert(false && "GenerateChunkResult doesnt have Chunk pointer");
			continue;
		}

		if (!IsChunkWithinUnloadRange(key) || chunks.contains(key)) {
			continue;
		}


		chunks[key] = std::move(genResult.chunk);


		Start_SkyLightTaskForNewChunk(*chunks[key]);
	}
}




void WorldThread::EnqueueMeshJob(Chunk& c) {

	ChunkJob job;
	job.coord = c.coord;
	job.snapshot = m_world.CreateMeshSnapshot(c);
	job.type = JobType::BUILD_MESH;

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


bool WorldThread::PopPendingDeleteMeshKey(ChunkCoord& out) {

	std::lock_guard<std::mutex> lock(pendingDeleteMeshMutex);

	if (m_pendingDeleteMeshKey.empty()) {
		return false;
	}

	out = std::move(m_pendingDeleteMeshKey.front());
	m_pendingDeleteMeshKey.pop_front();

	return true;

}


void WorldThread::PushPendingMesh(PendingMesh&& mesh) {
	if (!IsChunkWithinUnloadRange(mesh.key) ||
		!m_world.GetTargetChunkFromKey(mesh.key)) {
		return;
	}

	std::lock_guard<std::mutex> lock(pendingMeshMutex);

	m_pendingMeshData.push_back(std::move(mesh));

}



void WorldThread::Start_SkyLightTaskForNewChunk(Chunk& c) {

	LightTask task;
	task.lightType = LightType::SKY;
	task.sourceCoord = c.coord;


	int64_t wx = c.coord.x * Chunk::CHUNK_WIDTH;
	int64_t wz = c.coord.z * Chunk::CHUNK_DEPTH;


	m_lightEngine.InitializeSkylightForChunk(c);


	m_lightEngine.CreateSkylightLeakSeeds(c, task);

	
	if (task.bfs_queue.empty()) {
		
		FinishLightTask(task);//周囲チャンクをdirtyするために
		return;
	}
	



	
	m_lightTasks.push_back(task);
	
}



void WorldThread::ProcessLightTask(LightTask& task, int budget) {
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


	for (auto& key : task.dirtyChunks_light) {

		Chunk* c = m_world.GetTargetChunkFromKey(key);
		

		if (!c) {
			continue;
		}

		if (task.urgent) {
			MarkChunkUrgentDirty(*c);
		}
		else {
			MarkChunkDirty(*c);
		}

		MarkNeighborChunksDirty(c->coord);

	}

	for (auto& key : task.dirtyChunks_geometry) {

		Chunk* c = m_world.GetTargetChunkFromKey(key);


		if (!c) {
			continue;
		}

		if (task.urgent) {
			MarkChunkUrgentDirty(*c);
		}
		else {
			MarkChunkDirty(*c);
		}

		MarkNeighborChunksDirty(c->coord);

	}

	CheckBlockLightChangesForLightVolume(task);

}


uint64_t WorldThread::GetLightTaskDistance(const LightTask& task) const {
	const int64_t dx = std::abs(
		task.sourceCoord.x - m_lastStreamCoord.x
	);
	const int64_t dz = std::abs(
		task.sourceCoord.z - m_lastStreamCoord.z
	);

	return static_cast<uint64_t>(std::max(dx, dz));
}


void WorldThread::ProcLightTasks(std::chrono::steady_clock::time_point deadline) {
	if (m_lightTasks.empty() && m_urgentLightTasks.empty()) return;

	using clock = std::chrono::steady_clock;

	constexpr int CHECK_INTERVAL = 64;
	int processed = 0;

	auto processQueue = [this, deadline, &processed](
		std::deque<LightTask>& tasks
	) {
		while (!tasks.empty()) {
			auto target = std::min_element(
				tasks.begin(),
				tasks.end(),
				[this](const LightTask& a, const LightTask& b) {
					return GetLightTaskDistance(a) < GetLightTaskDistance(b);
				}
			);

			while (true) {
				LightTask& task = *target;
				ProcessLightTask(task, 1);
				++processed;

				const bool finished = task.phase == Phase::ADD ?
					task.bfs_queue.empty() :
					task.remove_queue.empty();

				if (finished) {
					FinishLightTask(task);
					tasks.erase(target);
					break;
				}

				if (processed % CHECK_INTERVAL == 0 &&
					clock::now() >= deadline) {
					return false;
				}
			}
		}

		return true;
	};

	if (!processQueue(m_urgentLightTasks)) return;
	if (clock::now() >= deadline) return;

	processQueue(m_lightTasks);
}




void WorldThread::QueueChunksToAutoSave() {


	auto& chunks = m_world.GetChunks();

	for (auto& [key, c_ptr] : chunks) {
		if (!c_ptr) continue;
		if (!c_ptr->dirtyToSave) continue;
		

		ChunkSaveData data;
		data.coord = c_ptr->coord;

		data.blocks.assign(
			c_ptr->blocks.begin(),
			c_ptr->blocks.end()
		);

		m_persistenceIO.RequestToSaveChunk(std::move(data));

		c_ptr->dirtyToSave = false;

	}

}



void WorldThread::DispatchDirtyMeshJobs() {

	auto& chunks = m_world.GetChunks();

	while (!m_dirtyMeshQueue.empty()) {

		auto entry = m_dirtyMeshQueue.top();
		m_dirtyMeshQueue.pop();

		auto it = chunks.find(entry.key);
		if (it != chunks.end()) {

			if (!it->second->dirty) {
				continue;
			}

			EnqueueMeshJob(*it->second);
			it->second->dirty = false;

		}
		else {

			continue;

		}


	}

	/*for (auto& [key, chunkPtr] : chunks) {
		
		if (!chunkPtr->dirty) continue;

		//if (chunkPtr->meshJobInFlight) continue;

		EnqueueMeshJob(*chunkPtr);

		chunkPtr->dirty = false;
	}*/

}


void WorldThread::DispatchOneDirtyMeshJob() {//TODO: 仕組みをunordered_setを使うやり方に変える後で

	auto& chunks = m_world.GetChunks();

	while (!m_dirtyMeshQueue.empty()) {

		auto entry = m_dirtyMeshQueue.top();
		m_dirtyMeshQueue.pop();

		auto it = chunks.find(entry.key);
		if (it != chunks.end()) {

			if (!it->second->dirty) {
				continue;
			}

			EnqueueMeshJob(*it->second);
			it->second->dirty = false;

			return;
		}
		else {

			continue;

		}


	}

	/*for (auto& [key, chunkPtr] : chunks) {
		
		if (!chunkPtr->dirty) continue;

		//if (chunkPtr->meshJobInFlight) continue;

		EnqueueMeshJob(*chunkPtr);

		chunkPtr->dirty = false;

		return;
	}*/

}


void WorldThread::MarkChunkDirty(Chunk& c) {


	c.dirty = true;
	c.dirtyToSave = true;

	const ChunkCoord relativeCoord{
		c.coord.x - m_lastStreamCoord.x,
		c.coord.z - m_lastStreamCoord.z
	};

	const auto rankIt = m_loadOffsetsRank.find(relativeCoord);

	if (rankIt == m_loadOffsetsRank.end()) {
		return;
	}


	const int priority =
		c.urgentUpdateMesh ?
		-1 :
		rankIt->second;

	m_dirtyMeshQueue.push({
		priority,
		c.coord
	});

}


void WorldThread::MarkChunkUrgentDirty(Chunk& c) {


	c.urgentUpdateMesh = true;

	MarkChunkDirty(c);
}

void WorldThread::MarkNeighborChunksDirty(ChunkCoord coord) {

	auto& chunks = m_world.GetChunks();

	for (int64_t x = coord.x - 1; x <= coord.x + 1; ++x) {
		if (x == coord.x) continue;

		ChunkCoord key = Index(x, coord.z);
		auto it = chunks.find(key);

		if (it == chunks.end() || !it->second) {

			//std::cout << "the chunk doesnt exist: " << x << ", " << cz << "\n";
			continue;
		}


		MarkChunkDirty(*it->second);
	}

	for (int64_t z = coord.z - 1; z <= coord.z + 1; ++z) {
		if (z == coord.z) continue;

		ChunkCoord key = Index(coord.x, z);
		auto it = chunks.find(key);

		if (it == chunks.end() || !it->second) {

			//std::cout << "the chunk doesnt exist: " << cx << ", " << z << "\n";
			continue;
		}

		

		MarkChunkDirty(*it->second);
	}
}

void WorldThread::MarkNeighborChunksUrgentDirty(ChunkCoord coord) {

	auto& chunks = m_world.GetChunks();


	for (int64_t x = coord.x - 1; x <= coord.x + 1; ++x) {
		if (x == coord.x) continue;

		ChunkCoord key = Index(x, coord.z);
		auto it = chunks.find(key);

		if (it == chunks.end() || !it->second) {
			continue;
		}

		it->second->urgentUpdateMesh = true;

		MarkChunkDirty(*it->second);

	}

	for (int64_t z = coord.z - 1; z <= coord.z + 1; ++z) {
		if (z == coord.z) continue;

		ChunkCoord key = Index(coord.x, z);
		auto it = chunks.find(key);

		if (it == chunks.end() || !it->second) {
			continue;
		}

		it->second->urgentUpdateMesh = true;

		MarkChunkDirty(*it->second);


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



RaycastHit WorldThread::RequestRaycast(const WorldPos& origin, const glm::vec3& dir, float distance) const {

	return m_world.Raycast(
		origin,
		dir,
		distance
	);

}


void WorldThread::ApplyPlayerStatus(float dt) {

	PlayerInput input;

	{
		std::lock_guard<std::mutex> lock(inputMutex);

		input = m_inputBuffer;

	}
	


	m_plr.Tick(dt, m_world, input);//player tick


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




void WorldThread::UpdatePlrSnapshot(std::chrono::steady_clock::time_point simTime) {

	PlayerSnapshot newSnap;

	newSnap.front = m_plr.GetFront();
	newSnap.pos = m_plr.GetEyePos();
	newSnap.right = m_plr.GetRight();
	newSnap.up = m_plr.GetUp();

	newSnap.simTime = simTime;

	if (!m_hasRenderSnap) {
		
		m_plrRenderSnap.previous = newSnap;
		m_plrRenderSnap.current = newSnap;

		m_hasRenderSnap = true;
		
	}
	else {

		m_plrRenderSnap.previous = m_plrRenderSnap.current;
		m_plrRenderSnap.current = newSnap;

	}


	m_exchanger.PublishPlrRenderSnap(m_plrRenderSnap);
}



void WorldThread::ProcCreateLightVSnap() {

	glm::i64vec3 center{};

	{
		std::lock_guard<std::mutex> lock(m_lightVolumeCenterMutex);

		center = m_lightVolumeCenter;
	}



	std::unique_ptr<LightVolumeSnapshot> snap = m_world.CreateLightVSnapshot(std::move(center));


	m_exchanger.PublishLightVolumeSnap(std::move(snap));

}





void WorldThread::ProcCreatePointLightsSnapshot() {


	auto snapshot = std::make_unique<PointLightsSnapshot>();
	auto& chunks = m_world.GetChunks();

	std::vector<PointLight> candidates;

	for (const auto& [key, chunkPtr] : chunks) {

		candidates.clear();

		const int64_t chunkMinX =
			key.x * static_cast<int64_t>(Chunk::CHUNK_WIDTH);

		const int64_t chunkMinZ =
			key.z * static_cast<int64_t>(Chunk::CHUNK_DEPTH);


		const int64_t chunkMaxX =
			chunkMinX + Chunk::CHUNK_WIDTH;

		const int64_t chunkMaxZ =
			chunkMinZ + Chunk::CHUNK_DEPTH;




		for (int x = -1; x <= 1; ++x) {
			for (int z = -1; z <= 1; ++z) {

				auto it = chunks.find(Index(key.x + x, key.z + z));
				if (it == chunks.end() || !it->second) continue;

				auto* c = it->second.get();

				for (auto& pl : c->pointLights) {

					const int64_t closestX = std::clamp(
						pl.position.x,
						chunkMinX,
						chunkMaxX
					);

					const int64_t closestZ = std::clamp(
						pl.position.z,
						chunkMinZ,
						chunkMaxZ
					);


					const double dxToChunk =
						static_cast<double>(
							pl.position.x - closestX
							);

					const double dzToChunk =
						static_cast<double>(
							pl.position.z - closestZ
							);


					const double distanceSquared =
						dxToChunk * dxToChunk +
						dzToChunk * dzToChunk;

					const double radiusSquared =
						static_cast<double>(pl.radius) *
						static_cast<double>(pl.radius);


					if (distanceSquared > radiusSquared) continue;

					candidates.push_back(pl);

				}

			}
		}


		std::sort(candidates.begin(), candidates.end(),
			[&](const PointLight& a, const PointLight& b) {

				auto distanceSquared = [&](const PointLight* light) {

					const int64_t closestX = std::clamp(
						light->position.x,
						chunkMinX,
						chunkMaxX
					);


					const int64_t closestZ = std::clamp(
						light->position.z,
						chunkMinZ,
						chunkMaxZ
					);

					const double dx =
						static_cast<double>(light->position.x - closestX);

					const double dz =
						static_cast<double>(light->position.z - closestZ);


					return dx * dx + dz * dz;
					};


				return distanceSquared(&a) < distanceSquared(&b);
			}
		);


		auto& lightsStruct = snapshot->pointLightsMap[key];
		size_t c = std::min<size_t>(candidates.size(), 16);



		lightsStruct.count = c;

		lightsStruct.pointLights.assign(
			candidates.begin(),
			candidates.begin() + c
		);

	}


	
	if (m_firstTimeCreatePlSnap.load()) {
		m_firstTimeCreatePlSnap.store(false);
	}


	m_exchanger.PublishPointLightSnap(std::move(snapshot));
}





void WorldThread::SetDebugStateFromDebug(const DebugActions& actions) {

	m_world.SetDebugStateFromDebug(actions);
}



void WorldThread::SetLightVolumeCenter(const glm::i64vec3& origin) {


	std::lock_guard<std::mutex> lock(m_lightVolumeCenterMutex);


	if (m_lightVolumeCenter == origin) return;

	m_lightVolumeDirty.store(true);
	m_lightVolumeCenter = origin;

}



bool WorldThread::IsInsideLightVolume(int64_t x, int64_t y, int64_t z) {

	using namespace LIGHT_VOLUME_SIZE;

	glm::i64vec3 o;

	{
		std::lock_guard<std::mutex> lock(m_lightVolumeCenterMutex);
		
		o = m_lightVolumeCenter;
	}

	return
		x >= o.x &&
		x <= o.x + LIGHT_VOLUME_WIDTH &&
		y >= o.y &&
		y <= o.y + LIGHT_VOLUME_HEIGHT &&
		z >= o.z &&
		z <= o.z + LIGHT_VOLUME_DEPTH;
	

}


WorldThreadDebugStats WorldThread::GetDebugStats() {
	WorldThreadDebugStats stats;
	stats.pipeline = m_chunkPipeline.GetDebugStats();

	stats.busyTimeNs = m_debugBusyTimeNs.load(std::memory_order_relaxed);
	stats.loopIterations = m_debugLoopIterations.load(std::memory_order_relaxed);
	stats.loadedChunks = m_debugLoadedChunks.load(std::memory_order_relaxed);
	stats.pendingChunkLoads =
		m_debugPendingChunkLoads.load(std::memory_order_relaxed);
	stats.normalLightTasks =
		m_debugNormalLightTasks.load(std::memory_order_relaxed);
	stats.urgentLightTasks =
		m_debugUrgentLightTasks.load(std::memory_order_relaxed);
	stats.dirtyMeshTasks =
		m_debugDirtyMeshTasks.load(std::memory_order_relaxed);
	stats.pendingMeshUploads =
		m_debugPendingMeshUploads.load(std::memory_order_relaxed);

	return stats;
}


void WorldThread::PublishDebugCounters() {
	m_debugLoadedChunks.store(
		m_world.GetChunks().size(),
		std::memory_order_relaxed
	);
	m_debugPendingChunkLoads.store(
		m_pendingChunkKeys.size(),
		std::memory_order_relaxed
	);
	m_debugNormalLightTasks.store(
		m_lightTasks.size(),
		std::memory_order_relaxed
	);
	m_debugUrgentLightTasks.store(
		m_urgentLightTasks.size(),
		std::memory_order_relaxed
	);
	m_debugDirtyMeshTasks.store(
		m_dirtyMeshQueue.size(),
		std::memory_order_relaxed
	);

	{
		std::lock_guard<std::mutex> lock(pendingMeshMutex);
		m_debugPendingMeshUploads.store(
			m_pendingMeshData.size(),
			std::memory_order_relaxed
		);
	}
}
