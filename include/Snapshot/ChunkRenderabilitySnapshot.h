#pragma once
#include <unordered_set>
#include <stdint.h>

#include "World/ChunkUtil.h"

using namespace ChunkUtil;


struct ChunkRenderabilitySnapshot {

	std::unordered_set<uint64_t> renderableChunks;//mesh‚ª‚ ‚échunk‚½‚¿

	bool IsRenderableChunk(int32_t cx, int32_t cz) const {

		return renderableChunks.contains(Index(cx, cz));

	}

};