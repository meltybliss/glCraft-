#pragma once
#include <stdint.h>
#include <memory>
#include "World/ChunkMeshSnapshot.h"

enum JobType {
	CREATE_CHUNK,
	GENERATE_TERRAIN,
	BUILD_MESH

};

enum class MeshBuildSource {
	INSTANCE_NEW_CHUNK,
	SNAPSHOT

};
struct ChunkJob {
	int32_t cx, cz;

	JobType type;
	MeshBuildSource meshSource;
	std::unique_ptr<ChunkMeshSnapshot> snapshot;//for BUILD_MESH
};
