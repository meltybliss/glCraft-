#pragma once
#include "Render/MeshData.h"

#include <memory>
#include <optional>
#include "World/ChunkCoord.h"


struct GeneratedChunkResult {
	ChunkCoord key;
	std::unique_ptr<Chunk> chunk;
};

struct MeshChunkResult {
	ChunkCoord key;
	std::optional<MeshData> meshData;
};
