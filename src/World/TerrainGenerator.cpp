#include "World/TerrainGenerator.h"

void TerrainGenerator::GenerateTerrain(Chunk& c) {

	for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x) {
		for (int z = 0; z < Chunk::CHUNK_DEPTH; ++z) {

			int64_t worldX = static_cast<int64_t>(c.cx) * Chunk::CHUNK_WIDTH + x;
			int64_t worldZ = static_cast<int64_t>(c.cz) * Chunk::CHUNK_DEPTH + z;
			int ground = GetHeight(worldX, worldZ);

			for (int y = Chunk::CHUNK_HEIGHT - 1; y >= 0; --y) {

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

				c.SetBlockForGenerator(x, y, z, b);
			}
		}
	}
	

	c.dirty = true;

}


int TerrainGenerator::GetHeight(int64_t worldX, int64_t worldZ) const {
	double n = m_perlinNoise.Noise(
		worldX * 0.01,
		worldZ * 0.01
	);

	return 64 + static_cast<int>(n * 68.0f);
}