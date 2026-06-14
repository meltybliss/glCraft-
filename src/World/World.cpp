#include "World/World.h"

int64_t floorDiv(int64_t a, int64_t b) {
	int64_t q = a / b;
	int64_t r = a % b;

	if (r != 0 && ((r < 0) != (b < 0))) {
		--q;
	}
	
	return q;

}

int floorMod(int64_t a, int b) {//-17, 16 -> 
	int64_t r = a % b;
	if (r < 0) {
		r += b;
	}

	return static_cast<int>(r);
}

unsigned int World::GetBlockGlobal(int64_t x, int64_t y, int64_t z) {
	int32_t cx = floorDiv(x, Chunk::CHUNK_WIDTH);
	int32_t cz = floorDiv(z, Chunk::CHUNK_DEPTH);

	int lx = floorMod(x, cx);
	int ly = y;
	int lz = floorMod(z, cz);

	auto it = chunks.find(Index(cx, cz));
	if (it == chunks.end() || !it->second) {
		return 0;
	}

	auto* c = it->second.get();

	return c->GetBlock(lx, ly, lz);

}


void World::SetBlockGlobal(int64_t x, int64_t y, int64_t z, BlockType b) {
	int32_t cx = floorDiv(x, Chunk::CHUNK_WIDTH);
	int32_t cz = floorDiv(z, Chunk::CHUNK_DEPTH);

	int lx = floorMod(x, cx);
	int ly = y;
	int lz = floorMod(z, cz);

	auto it = chunks.find(Index(cx, cz));
	if (it == chunks.end() || !it->second) {
		return;
	}

	auto* c = it->second.get();

	c->SetBlock(lx, ly, lz, b);
}