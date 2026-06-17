#pragma once
#include <stdint.h>
#include <optional>
#include "World/ChunkMeshSnapshot.h"

enum JobType {
	CREATE_CHUNK,
	GENERATE_TERRAIN,
	BUILD_MESH

};
struct ChunkJob {
	int32_t cx, cz;

	JobType type;
	std::optional<ChunkMeshSnapshot> snapshot;//for BUILD_MESH
};