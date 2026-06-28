#pragma once
#include "Render/MeshData.h"

#include <memory>
#include <optional>


struct GeneratedChunkResult {
	uint64_t key;
	std::unique_ptr<Chunk> chunk;
};

struct MeshChunkResult {
	bool wasNewChunk = false;
	uint64_t key;
	std::optional<MeshData> meshData;
};