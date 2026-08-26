#pragma once
#include <unordered_set>
#include <stdint.h>

#include "World/ChunkUtil.h"

using namespace ChunkUtil;


struct ChunkRenderabilitySnapshot {

	std::unordered_set<ChunkCoord, ChunkCoordHash> renderableChunks;//meshがあるchunkたち

	bool IsRenderableChunk(ChunkCoord coord) const {

		return renderableChunks.contains(coord);

	}

};
