#pragma once
#include "World/Chunk.h"
#include "World/ChunkKey.h"
#include "World/ChunkResult.h"
#include "World/ChunkPipeline.h"
#include "Render/PendingMesh.h"
#include "World/RaycastHit.h"
#include <unordered_map>
#include <memory>
#include <unordered_set>
#include <stdint.h>
#include <deque>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using ChunkMapKey = uint64_t;
using namespace ChunkKey;

struct Camera;
class ChunkPipeline;

class World {
public:
	World();
	~World();

	using ChunkMap = std::unordered_map<ChunkMapKey, std::unique_ptr<Chunk>>;

	[[nodiscard]] unsigned int GetBlockGlobal(int64_t x, int64_t y, int64_t z) const;

	void SetBlockGlobal(int64_t x, int64_t y, int64_t z, BlockType b);
	void SetBlockGlobal_User(int64_t x, int64_t y, int64_t z, BlockType b);

	[[nodiscard]] ChunkMap& GetChunks() {
		return chunks;
	}

	[[nodiscard]] const ChunkMap& GetChunks() const {
		return chunks;
	}

	[[nodiscard]] Chunk* GetTargetChunk(int32_t cx, int32_t cz) {
		uint64_t key = Index(cx, cz);
		auto it = chunks.find(key);

		if (it == chunks.end()) return nullptr;

		return it->second.get();
	}

	[[nodiscard]] Chunk* GetTargetChunkFromKey(uint64_t key) {
		
		auto it = chunks.find(key);

		if (it == chunks.end()) return nullptr;

		return it->second.get();
	}

	[[nodiscard]] const Chunk* GetTargetChunkFromKey(uint64_t key) const {

		auto it = chunks.find(key);

		if (it == chunks.end()) return nullptr;

		return it->second.get();
	}

	[[nodiscard]] const Chunk* GetTargetChunk(int32_t cx, int32_t cz) const {
		uint64_t key = Index(cx, cz);
		auto it = chunks.find(key);

		if (it == chunks.end()) return nullptr;

		return it->second.get();
	}

	[[nodiscard]] RaycastHit Raycast(const glm::vec3& origin, const glm::vec3& dir, float distance) const;
	
	void Tick(float dt, const Camera& cam);
	void UpdateChunksAround(const Camera& cam);


	void DebugChunkInfo();

	bool PopPendingMeshData(PendingMesh& out);
	void EnqueuePendingChunkKey(uint64_t key) {
	
		m_pendingChunkKeys.insert(key);
	}
	void EnqueueMeshJobFrom_Outside(Chunk& c);
	
	static int Get_UNLOAD_DISTANCE() {
		return UNLOAD_CHUNKS_DISTANCE;
	}

	static int Get_LOAD_DISTANCE() {
		return LOAD_CHUNKS_DISTANCE;
	}

	std::unique_ptr<ChunkMeshSnapshot> CreateMeshSnapshot(Chunk& c);
private:

	static constexpr int LOAD_CHUNKS_DISTANCE = 12;
	static constexpr int UNLOAD_CHUNKS_DISTANCE = 14;

	static constexpr int MAX_CHUNK_CREATE_PER_TICK = 8;
	static constexpr int MAX_CHUNK_DESTROY_PER_TICK = 10;

private:

	ChunkMap chunks;
	ChunkPipeline m_chunkPipeline;

	std::deque<PendingMesh> m_pendingMeshData;//to collect and load its meshData in order
	std::unordered_set<uint64_t> m_pendingChunkKeys;//to avoid submitting instructions for submitted chunks

	int32_t m_lastStreamCx = std::numeric_limits<int32_t>::max();
	int32_t m_lastStreamCz = std::numeric_limits<int32_t>::max();

	int32_t curCx = 0;
	int32_t curCz = 0;

private:

	void ProcessChunkResult();

	void MarkNeighborChunksDirty(const int32_t cx, const int32_t cz);

};