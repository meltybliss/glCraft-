#pragma once
#include "World/Chunk.h"

#include <memory>

struct ChunkResult {
	uint64_t key;

	std::unique_ptr<Chunk> chunk;
	MeshData meshData;
};