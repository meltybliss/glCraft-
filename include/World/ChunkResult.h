#pragma once
#include "World/Chunk.h"

#include <memory>
#include <optional>

struct ChunkResult {
	uint64_t key;

	std::optional<MeshData> meshData;
	std::unique_ptr<Chunk> chunk;
};