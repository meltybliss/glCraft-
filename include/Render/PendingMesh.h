#pragma once

#include "Render/StagedMeshData.h"
#include "World/ChunkCoord.h"

struct PendingMesh {
	StagedMeshData stagedMesh;
	ChunkCoord key;

};
