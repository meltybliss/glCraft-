#pragma once
#include "World/Chunk.h"
#include "World/ChunkUtil.h"
#include "World/ChunkResult.h"
#include "Render/PendingMesh.h"
#include "World/RaycastHit.h"
#include "World/ChunkMeshSnapshot.h"

#include "Util/ThreadSafeLogUtils.h"
#include <unordered_map>
#include <memory>
#include <unordered_set>
#include <stdint.h>
#include <deque>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using ChunkMapKey = uint64_t;
using namespace ChunkUtil;

struct Camera;
class ChunkPipeline;

class World {
public:
	
	World() = default;

	using ChunkMap = std::unordered_map<ChunkMapKey, std::unique_ptr<Chunk>>;

	[[nodiscard]] unsigned int GetBlockGlobal(int64_t x, int64_t y, int64_t z) const;
	[[nodiscard]] uint8_t GetBlockLightGlobal(int64_t x, int64_t y, int64_t z) const;
	[[nodiscard]] uint8_t GetSkyLightGlobal(int64_t x, int64_t y, int64_t z) const;

	void SetBlockGlobal(int64_t x, int64_t y, int64_t z, BlockType b);
	void SetBlockGlobal_User(int64_t x, int64_t y, int64_t z, BlockType b);
	bool SetBlockLightGlobal(int64_t x, int64_t y, int64_t z, uint8_t level);
	bool SetSkyLightGlobal(int64_t x, int64_t y, int64_t z, uint8_t level);

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
	
	void DebugChunkInfo();


	std::unique_ptr<ChunkMeshSnapshot> CreateMeshSnapshot(Chunk& c);

	std::unique_ptr<ChunkMeshSnapshot> CreateMeshSnapshotFromKey(uint64_t key);


	void MarkNeighborChunksDirty(const int32_t cx, const int32_t cz);
	void MarkNeighborChunksUrgentDirty(const int32_t cx, const int32_t cz);

	void MarkChunkDirty(Chunk& c) {
		c.dirty = true;
	}
	void MarkChunkUrgentDirty(Chunk& c) {
		c.dirty = true;
		c.urgentUpdateMesh = true;
		
	}


	bool CanCollideBlock(int64_t x, int64_t y, int64_t z) const {
		BlockType block = static_cast<BlockType>(
			GetBlockGlobal(x, y, z)
		);


		return block != BlockType::AIR && block != BlockType::TORCH;
	}

private:

private:

	ChunkMap chunks;
	
};