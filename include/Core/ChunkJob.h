#pragma once
#include <stdint.h>
#include <memory>
#include "World/ChunkMeshSnapshot.h"

enum JobType {
	CREATE_CHUNK,
	GENERATE_TERRAIN,
	BUILD_MESH

};

struct ChunkJob {
	int32_t cx, cz;
	bool urgent = false;
	bool isNewChunk = false;

	JobType type;
	std::unique_ptr<ChunkMeshSnapshot> snapshot;//for BUILD_MESH
};
