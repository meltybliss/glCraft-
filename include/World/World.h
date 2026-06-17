#pragma once
#include "World/Chunk.h"
#include <unordered_map>
#include <memory>
#include <stdint.h>


using ChunkKey = uint64_t;

struct Camera;

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

	void Tick(float dt, const Camera& cam);
	void UpdateChunksAround(const Camera& cam);


	void DebugChunkInfo();

private:

	static constexpr int LOAD_CHUNKS_DISTANCE = 12;
	static constexpr int UNLOAD_CHUNKS_DISTANCE = 14;

	static constexpr int MAX_CHUNK_CREATE_PER_TICK = 4;
	static constexpr int MAX_CHUNK_DESTROY_PER_TICK = 5;

private:

	ChunkMap chunks;


private:


	static uint64_t Index(int32_t cx, int32_t cz) {
		uint64_t x = static_cast<uint32_t>(cx);
		uint64_t z = static_cast<uint32_t>(cz);

		return (x << 32) | z;
	}
};