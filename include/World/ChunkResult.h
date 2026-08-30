#pragma once
#include "Render/MeshData.h"

#include <memory>
#include <optional>
#include "World/ChunkCoord.h"

#include "Render/StagedMeshData.h"
#include "Render/MeshGeneration.h"

struct GeneratedChunkResult {
	ChunkCoord key;
	std::unique_ptr<Chunk> chunk;
};

struct MeshChunkResult {
	ChunkCoord key;
	std::optional<StagedMeshData> stagedMesh;
	MeshGenerationStamp generation;
};
