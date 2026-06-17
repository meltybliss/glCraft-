#pragma once
#include "World/Chunk.h"

#include <memory>
#include <optional>

struct ChunkResult {
	uint64_t key;

	std::unique_ptr<Chunk> chunk;
	std::optional<MeshData> meshData;
};