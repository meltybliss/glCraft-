#pragma once
#include "World/Chunk.h"
#include <memory>

struct MeshResult {
	std::unique_ptr<Chunk> chunk;
	MeshData meshData;
};