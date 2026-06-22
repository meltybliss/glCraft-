#pragma once
#include "Core/ChunkJob.h"
#include "World/Chunk.h"
#include "World/ChunkKey.h"
#include "World/ChunkMeshSnapshot.h"
#include "World/ChunkResult.h"
#include "World/TerrainGenerator.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <deque>
#include <unordered_map>
#include <condition_variable>
#include <stdint.h>
#include <memory>

using namespace ChunkKey;

class World;

class ChunkPipeline {
public:
	explicit ChunkPipeline(World* w, uint64_t seed) : m_world(w), m_terrainGen(seed) {}

	void StartWorkerThread();
	void StopWorkerThread();

	void EnqueueJob(ChunkJob&& job);

	bool PopFrontResult(ChunkResult& out);

private:
	void StartLoop();

private:

	static constexpr int MAX_CHUNK_CREATE_PER_TICK = 4;
	static constexpr int MAX_CHUNK_DESTROY_PER_TICK = 5;

	static constexpr int MAX_CHUNK_TERRAIN_PER_TICK = 5;
	static constexpr int MAX_CHUNK_MESH_PER_TICK = 5;

private:
	World* m_world = nullptr;
	TerrainGenerator m_terrainGen;

	std::thread workerThread;
	std::atomic<bool> runningWorker = false;

	std::mutex jobsMutex;
	std::mutex resultMutex;

	std::condition_variable workerCv;

	std::deque<ChunkJob> m_jobQueue;
	std::deque<ChunkResult> m_chunkResult;
	std::unordered_map<uint64_t, std::unique_ptr<Chunk>> m_buildingChunks;
private:

	void ProcessJob(ChunkJob&& job);
	
};