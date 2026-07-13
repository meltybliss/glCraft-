#pragma once

#include <stdint.h>


struct ChunkDirtyEntry {

	int priority;
	uint64_t key;

	bool operator<(const ChunkDirtyEntry& other) const {
		return priority > other.priority;
	}

};


