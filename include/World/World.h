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
private:

	static constexpr int LOAD_CHUNKS_DISTANCE = 2;
	static constexpr int UNLOAD_CHUNKS_DISTANCE = 4;

private:

	ChunkMap chunks;


private:
	

	static uint64_t Index(int32_t cx, int32_t cz) {
		return (static_cast<uint64_t>(cx) << 32) | cz;
	}
};