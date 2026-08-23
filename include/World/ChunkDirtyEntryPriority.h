#pragma once

#include <stdint.h>
#include "World/ChunkCoord.h"


struct ChunkDirtyEntry {

	int priority;
	ChunkCoord key;

	bool operator<(const ChunkDirtyEntry& other) const {
		return priority > other.priority;
	}

};


