#include "World/TerrainGenerator.h"

void TerrainGenerator::GenerateTerrain(Chunk* c) {

	int ground = Chunk::CHUNK_HEIGHT / 2;

	for (int y = Chunk::CHUNK_HEIGHT-1; y >= 0; --y) {
		for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x) {
			for (int z = 0; z < Chunk::CHUNK_DEPTH; ++z) {
				BlockType b = BlockType::AIR;

				if (y == ground) {
					b = BlockType::GRASS;
				}
				else if (y < ground && y > ground - 5) {
					b = BlockType::DIRT;
				}
				else if (y <= ground - 5) {
					b = BlockType::STONE;
				}

				c->SetBlockForGenerator(x, y, z, b);
			}
		}
	}

	c->dirty = true;

}