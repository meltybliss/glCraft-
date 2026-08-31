#pragma once
#include <unordered_set>
#include "World/Chunk.h"


struct MeshUpdateContext {

	std::unordered_set<ChunkCoord, ChunkCoordHash> invalidatedChunks;

	void BeginBatch() {
		invalidatedChunks.clear();
	}

};



inline void InvalidateMeshGeneration(Chunk& c, MeshUpdateContext& context) {

	auto [it, inserted] = context.invalidatedChunks.insert(c.coord);
	if (!inserted) return;


	c.AdvanceMeshGeneration();


}
