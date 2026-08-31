#pragma once

#include "Render/StagedMeshData.h"
#include "Render/MeshGeneration.h"
#include "World/ChunkCoord.h"

struct PendingMesh {
	StagedMeshData stagedMesh;
	ChunkCoord key;
	MeshGenerationStamp generation;

};
