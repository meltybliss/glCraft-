#pragma once
#include "World.h"
#include "Chunk.h"
#include "WorldCommand.h"
#include "LightTask.h"
#include "LightEngine.h"
#include "ChunkPipeline.h"
#include "Gameplay/Player.h"
#include "Gameplay/PlayerInput.h"
#include "Gameplay/PlayerSnapshot.h"
#include "Util/ThreadSafeLogUtils.h"
#include "ChunkDirtyEntryPriority.h"
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <queue>

struct ChunkOffset {
	int32_t dx = 0;
	int32_t dz = 0;
};


class WorldThread {
public:

	WorldThread() : m_chunkPipeline(&m_world, 114514) {}

	void StartLoop();
	void StopLoop();

	void SubmitEditBlock(
		int64_t worldX,
		int64_t worldY,
		int64_t worldZ,
		BlockType b

	);

	void SetDesiredStreamCenter(
		int32_t cx,
		int32_t cz
	);

	bool PopPendingMeshData(PendingMesh& out);
	bool PopPendingDeleteMeshKey(uint64_t& out);

	static int Get_UNLOAD_DISTANCE() {

		return UNLOAD_CHUNKS_DISTANCE;
	}

	static int Get_LOAD_DISTANCE() {
		return LOAD_CHUNKS_DISTANCE;
	}

	void SetInput(PlayerInput&& input) {
		{
			std::lock_guard<std::mutex> lock(inputMutex);

			m_inputBuffer = input;
		}

		m_hasSettedInput.store(true);

		Wake();
	}


	void AddMouseDelta(float xoffset, float yoffset);

	RaycastHit RequestRaycast(const glm::vec3& origin, const glm::vec3& dir, float distance) const;


	void Rebuild_allChunks();

	void Debug_CurStreamCenter();

	[[nodiscard]] PlayerSnapshot GetPlrSnapshot() {
		std::lock_guard<std::mutex> lock(snapshotMutex);

		return m_plrSnapshot;
	}
private:
	World m_world;
	ChunkPipeline m_chunkPipeline;
	LightEngine m_lightEngine;

	PlayerInput m_inputBuffer{};


	PlayerSnapshot m_plrSnapshot{};

	Player m_plr;

	std::thread worldThread;
	std::condition_variable worldCv;

	
	bool requestedToWake = false;

	int32_t m_streamCx = std::numeric_limits<int32_t>::max();
	int32_t m_streamCz = std::numeric_limits<int32_t>::max();
	int32_t m_lastStreamCx = std::numeric_limits<int32_t>::max();
	int32_t m_lastStreamCz = std::numeric_limits<int32_t>::max();

	std::atomic<bool> runningWorldThread;

	bool m_streamNeedsUpdate = false;

	std::atomic<bool> m_hasSettedDesireStreamC = false;
	std::atomic<bool> m_hasSettedInput = false;
	std::atomic<bool> m_hasMovedMouse = false;

	float m_xoffsetBuffer = 0.f;
	float m_yoffsetBuffer = 0.f;

	std::mutex snapshotMutex;
	std::mutex streamCenterMutex;
	std::mutex commandMutex;
	std::mutex pendingMeshMutex;
	std::mutex waitMutex;
	std::mutex pendingDeleteMeshMutex;
	std::mutex inputMutex;
	std::mutex offsetMutex;
	

	std::deque<WorldCommand> m_commands;

	std::deque<LightTask> m_lightTasks;
	std::deque<LightTask> m_urgentLightTasks;


	std::deque<PendingMesh> m_pendingMeshData;//to collect and load its meshData in order
	std::unordered_set<uint64_t> m_pendingChunkKeys;//to avoid submitting instructions for submitted chunks to create and generate Terrain
	

	std::deque<uint64_t> m_pendingDeleteMeshKey;

	std::priority_queue<ChunkDirtyEntry> m_dirtyMeshQueue;

	std::vector<ChunkOffset> m_loadOffsets;
	std::unordered_map<uint64_t, int> m_loadOffsetsRank;//dxÇ∆dzÇçáëÃÇ≥ÇπÇΩkey

	size_t m_nextLoadOffset = 0;
private:

	static constexpr int LOAD_CHUNKS_DISTANCE = 12;
	static constexpr int UNLOAD_CHUNKS_DISTANCE = 14;

	static constexpr int MAX_CHUNK_CREATE_PER_TICK = 8;
	static constexpr int MAX_CHUNK_DESTROY_PER_TICK = 10;


	static constexpr int MAX_LIGHT_PROPAGATE_BFS_PER_TICK = 2000;
private:

	void ProcCommands();
	void ProcOneCommand();

	void ProcChunkResults();
	void ProcOneChunkResult();

	void ApplyCommand(WorldCommand& cmd);
	void ApplyEditBlock(
		int64_t x,
		int64_t y,
		int64_t z,
		BlockType b
	);

	void ApplyStreamCenter();

	void ApplyPlayerStatus(float dt);

	
	void ApplyMouseMovement();

	void BuildLoadOffsets();
	void UpdateChunksAround_step();
	bool RequestOneMissingChunkAround();
	bool RequestEraseOneChunkAround();

	bool HasChunkToErase();
	bool HasChunkToCreate();

	void UpdateChunksAround();
	
	void TickSimulation(float dt);
	void TickBackground(std::chrono::steady_clock::time_point deadline);


	void EnqueueMeshJob(Chunk& c);


	void UpdatePlrSnapshot();

	void PushPendingMesh(PendingMesh& mesh);
	
	void Start_BlockLightTask(
		int64_t x,
		int64_t y,
		int64_t z,
		uint8_t level,
		bool urgent = false
	);
	void Start_RemoveBlockLightTask(
		int64_t x,
		int64_t y,
		int64_t z,
		bool urgent = false
	);

	void Start_RemoveBlockLightTask_WithEmissionTask(
		int64_t x,
		int64_t y,
		int64_t z,
		uint8_t emissionAfterRemove,
		bool urgent = false
	);

	void Start_RemoveSkyLightTask(
		int64_t x,
		int64_t y,
		int64_t z,
		bool urgent = false
	);

	void Start_SkyLightTaskForNewChunk(Chunk& c);
	void Start_BlockLightTaskFromNeighbors(
		int64_t x,
		int64_t y,
		int64_t z,
		bool urgent = false
	);

	void Start_SkyLightTask(
		int64_t x,
		int64_t y,
		int64_t z,
		uint8_t level,
		bool urgent
	);

	void Add_SkylightTask(
		int64_t x,
		int64_t y,
		int64_t z,
		bool urgent
	);

	void ProcLightTasks();
	void ProcessLightTask(LightTask& task, int& budget);
	void FinishLightTask(LightTask& task);

	void DispatchDirtyMeshJobs();
	void DispatchOneDirtyMeshJob();

	void MarkChunkDirty(Chunk& c);
	void MarkNeighborChunksDirty(const int32_t cx, const int32_t cz);
	void MarkNeighborChunksUrgentDirty(const int32_t cx, const int32_t cz);

	void MarkChunkUrgentDirty(Chunk& c);

	bool HasImmediateTask();
	void Wake();
};