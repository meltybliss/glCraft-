#pragma once
#include <unordered_set>
#include <stdint.h>

#include "World/ChunkUtil.h"

using namespace ChunkUtil;


struct ChunkRenderabilitySnapshot {

	std::unordered_set<ChunkCoord, ChunkCoordHash> renderableChunks;//mesh‚ª‚ ‚échunk‚½‚¿

	bool IsRenderableChunk(ChunkCoord coord) const {

		return renderableChunks.contains(coord);

	}

};
