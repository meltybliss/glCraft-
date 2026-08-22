#pragma once
#include "World.h"
#include "Chunk.h"
#include "WorldCommand.h"
#include "LightTask.h"
#include "LightEngine.h"
#include "ChunkPipeline.h"
#include "Gameplay/Player.h"
#include "Gameplay/PlayerInput.h"
#include "Util/ThreadSafeLogUtils.h"
#include "ChunkDirtyEntryPriority.h"
#include "Debugs/DebugActions.h"
#include "Persistence/WorldSaveData.h"
#include "Snapshot/SnapshotExchanger.h"
#include "Debugs/DebugSettings.h"
#include "WorldConfig.h"
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <deque>
#include <condition_variable>
#include <queue>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


using namespace WorldConfig;

struct ChunkOffset {
	int32_t dx = 0;
	int32_t dz = 0;
};


class PersistenceIO;


class WorldThread {
public:

	WorldThread(SnapshotExchanger& exchanger, PersistenceIO& pIO)
		: m_chunkPipeline(&m_world), m_persistenceIO(pIO), m_exchanger(exchanger)  {}


	void SetWorldSelectionResult(WorldSelectionResult& info);

	void CreateNewWorld(uint64_t seed);
	void ApplyLoadedWorld(WorldSaveData& saveData);

	void StartThread();
	void StopThread();

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

	RaycastHit RequestRaycast(const WorldPos& origin, const glm::vec3& dir, float distance) const;


	void Rebuild_allChunks();


	[[nodiscard]] World* GetWorldPtr() {
		return &m_world;
	}

	[[nodiscard]] const World* GetWorldPtr() const {
		return &m_world;
	}

	
	void SetDebugStateFromDebug(const DebugActions& actions);


	void SetLightVolumeCenter(const glm::i64vec3& origin);
	WorldThreadDebugStats GetDebugStats();

private:
	

	World m_world;
	ChunkPipeline m_chunkPipeline;
	LightEngine m_lightEngine;
	SnapshotExchanger& m_exchanger;
	PersistenceIO& m_persistenceIO;

	PlayerInput m_inputBuffer{};


	PlayerRenderSnapshot m_plrRenderSnap{};
	bool m_hasRenderSnap = false;

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


	std::atomic<bool> m_firstTimeCreatePlSnap;


	using Clock = std::chrono::steady_clock;

	Clock::time_point m_nextAutoSaveTime;

	static constexpr auto AUTO_SAVE_INTERVAL =
		std::chrono::seconds(30);



	std::mutex m_lightVolumeCenterMutex;
	std::atomic<bool> m_lightVolumeDirty = false;
	std::atomic<bool> m_pointLightDirty = false;

	std::atomic<uint64_t> m_debugBusyTimeNs = 0;
	std::atomic<uint64_t> m_debugLoopIterations = 0;
	std::atomic<size_t> m_debugLoadedChunks = 0;
	std::atomic<size_t> m_debugPendingChunkLoads = 0;
	std::atomic<size_t> m_debugNormalLightTasks = 0;
	std::atomic<size_t> m_debugUrgentLightTasks = 0;
	std::atomic<size_t> m_debugDirtyMeshTasks = 0;
	std::atomic<size_t> m_debugPendingMeshUploads = 0;

	glm::i64vec3 m_lightVolumeCenter{0};



private:
 

	static constexpr int MAX_CHUNK_CREATE_PER_TICK = 8;
	static constexpr int MAX_CHUNK_DESTROY_PER_TICK = 10;


	static constexpr int MAX_LIGHT_PROPAGATE_BFS_PER_TICK = 2000;


	static constexpr int MAX_CHUNK_TERRAIN_PER_TICK = 7;
	static constexpr int MAX_CHUNK_MESH_PER_TICK = 8;

private:

private:

	void ProcCommands();
	void ProcOneCommand();

	void ProcChunkResults();
	void ProcOneChunkResult();

	void ProcOne_Disk_ChunkLoadResult();

	void ProcCreateLightVSnap();
	void ProcCreatePointLightsSnapshot();

	void ApplyCommand(WorldCommand& cmd);
	void ApplyEditBlock(
		int64_t x,
		int64_t y,
		int64_t z,
		BlockType b
	);

	void QueueChunksToAutoSave();
	void ForcedSave_World();
	void ForcedSave_Chunks();

	void CheckAutoSave();


	WorldSaveData CreateWorldSaveData() const;

	void ApplyStreamCenter();

	void ApplyPlayerStatus(float dt);

	
	void ApplyMouseMovement();

	void BuildLoadOffsets();
	void UpdateChunksAround_step();
	bool RequestOneMissingChunkAround();
	bool RequestEraseOneChunkAround();

	bool HasChunkToErase();
	bool HasChunkToCreate();
	bool IsChunkWithinUnloadRange(int32_t cx, int32_t cz) const;
	void QueueMeshDeletion(uint64_t key);
	void RebuildDirtyMeshQueuePriorities();

	void UpdateChunksAround();

	void UpdateDayNightState(float dt);
	
	void TickSimulation(float dt, std::chrono::steady_clock::time_point simTime);
	void TickBackground(std::chrono::steady_clock::time_point deadline);


	void EnqueueMeshJob(Chunk& c);


	void UpdateDayNightSnap();
	void UpdatePlrSnapshot(std::chrono::steady_clock::time_point simTime);

	void PushPendingMesh(PendingMesh&& mesh);
	
	void Start_BlockLightTask(
		int64_t x,
		int64_t y,
		int64_t z,
		uint8_t level,
		const glm::vec3& color,
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
		int level,
		bool urgent
	);

	void Add_SkylightTask(
		int64_t x,
		int64_t y,
		int64_t z,
		bool urgent
	);

	void ProcLightTasks(std::chrono::steady_clock::time_point deadline);
	void ProcessLightTask(LightTask& task, int budget);
	void FinishLightTask(LightTask& task);
	uint64_t GetLightTaskDistance(const LightTask& task) const;

	void DispatchDirtyMeshJobs();
	void DispatchOneDirtyMeshJob();

	void MarkChunkDirty(Chunk& c);
	void MarkNeighborChunksDirty(const int32_t cx, const int32_t cz);
	void MarkNeighborChunksUrgentDirty(const int32_t cx, const int32_t cz);

	void MarkChunkUrgentDirty(Chunk& c);

	bool HasImmediateTask();
	void Wake();
	void PublishDebugCounters();


	bool IsInsideLightVolume(int64_t x, int64_t y, int64_t z);

	void CheckBlockLightChangesForLightVolume(LightTask& task);

};
