#pragma once
#include "MeshData.h"
#include "World/ChunkCoord.h"

struct PendingMesh {
	MeshData meshData;
	ChunkCoord key;

};
