#pragma once
#include "World.h"
#include "Chunk.h"
#include "WorldCommand.h"
#include "LightTask.h"
#include "LightEngine.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <deque>
class WorldThread {
public:


	void StartLoop();
	void StopLoop();

	void SubmitEditBlock(
		int32_t worldX,
		int32_t worldY,
		int32_t worldZ,
		BlockType b

	);

	void SetDesiredStreamCenter(
		int32_t cx,
		int32_t cz
	);

private:
	World m_world;
	ChunkPipeline m_chunkPipeline;
	LightEngine m_lightEngine;

	std::thread worldThread;
	
	int32_t m_streamCx = 0;
	int32_t m_streamCz = 0;
	int32_t m_lastStreamCx = std::numeric_limits<int32_t>::max();
	int32_t m_lastStreamCz = std::numeric_limits<int32_t>::max();

	std::atomic<bool> runningWorldThread;

	bool m_streamNeedsUpdate = false;

	std::mutex streamCenterMutex;
	std::mutex commandMutex;
	std::mutex pendingMeshMutex;

	std::deque<WorldCommand> m_commands;
	std::deque<LightTask> m_lightTasks;


	std::deque<PendingMesh> m_pendingMeshData;//to collect and load its meshData in order
	std::unordered_set<uint64_t> m_pendingChunkKeys;//to avoid submitting instructions for submitted chunks

private:

	static constexpr int LOAD_CHUNKS_DISTANCE = 12;
	static constexpr int UNLOAD_CHUNKS_DISTANCE = 14;

	static constexpr int MAX_CHUNK_CREATE_PER_TICK = 8;
	static constexpr int MAX_CHUNK_DESTROY_PER_TICK = 10;


	static constexpr int MAX_LIGHT_PROPAGATE_BFS_PER_TICK = 5000;
private:

	void ProcCommands();
	void ProcChunkResults();

	void ApplyCommand(WorldCommand& cmd);
	void ApplyEditBlock(
		int32_t x,
		int32_t y,
		int32_t z,
		BlockType b
	);

	void ApplyStreamCenter();

	void UpdateChunksAround();
	void Tick();

	void EnqueueMeshJob(Chunk& c);


	void PushPendingMesh(PendingMesh& mesh);
	bool PopPendingMeshData(PendingMesh& out);


	void Start_BlockLightTask(
		int32_t x,
		int32_t y,
		int32_t z, 
		uint8_t level);
	void Start_SkyLightTaskForNewChunk(Chunk& c);
	void ProcLightTasks();
};