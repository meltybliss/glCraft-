#pragma once
#include "World/Chunk.h"
#include "World/ChunkKey.h"
#include "World/ChunkResult.h"
#include "World/ChunkPipeline.h"
#include "Render/PendingMesh.h"
#include <unordered_map>
#include <memory>
#include <stdint.h>
#include <deque>


using ChunkKey = uint64_t;
using namespace ChunkKey;

struct Camera;
class ChunkPipeline;

class World {
public:
	using ChunkMap = std::unordered_map<ChunkKey, std::unique_ptr<Chunk>>;

	[[nodiscard]] unsigned int GetBlockGlobal(int64_t x, int64_t y, int64_t z) const;

	void SetBlockGlobal(int64_t x, int64_t y, int64_t z, BlockType b);

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

	[[nodiscard]] Chunk* GetTargetChunkFromKey(uint32_t key) {
		
		auto it = chunks.find(key);

		if (it == chunks.end()) return nullptr;

		return it->second.get();
	}

	[[nodiscard]] const Chunk* GetTargetChunkFromKey(uint32_t key) const {

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

	void Tick(float dt, const Camera& cam);
	void UpdateChunksAround(const Camera& cam);


	void DebugChunkInfo();

	bool PopPendingMeshData(PendingMesh& out);
private:

	static constexpr int LOAD_CHUNKS_DISTANCE = 12;
	static constexpr int UNLOAD_CHUNKS_DISTANCE = 14;

	static constexpr int MAX_CHUNK_CREATE_PER_TICK = 4;
	static constexpr int MAX_CHUNK_DESTROY_PER_TICK = 5;

private:

	ChunkMap chunks;
	ChunkPipeline m_chunkPipeline;

	std::deque<PendingMesh> m_pendingMeshData;

private:

	void ProcessChunkResult();

};