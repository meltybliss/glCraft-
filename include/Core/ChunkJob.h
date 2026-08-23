#pragma once
#include <stdint.h>
#include <memory>
#include "Snapshot/ChunkMeshSnapshot.h"
#include "World/ChunkCoord.h"

enum JobType {
	CREATE_CHUNK,
	GENERATE_TERRAIN,
	BUILD_MESH

};

struct ChunkJob {
	ChunkCoord coord;
	bool urgent = false;

	JobType type;
	std::unique_ptr<ChunkMeshSnapshot> snapshot;//for BUILD_MESH
};
