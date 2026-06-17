#pragma once
#include <stdint.h>


enum JobType {
	CREATE_CHUNK,
	GENERATE_TERRAIN,
	BUILD_MESH

};
struct ChunkJob {
	int32_t cx, cz;

	JobType type;
};