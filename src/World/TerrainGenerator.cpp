#include "World/TerrainGenerator.h"
#include <algorithm>

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
	constexpr double frequency = 0.01;
	constexpr double baseHeight = 64.0;
	constexpr double amplitude = 18.0;

	double x = static_cast<double>(worldX);
	double z = static_cast<double>(worldZ);

	const double n = m_perlinNoise.Noise(
		x * frequency,
		z * frequency
	) * amplitude;

	const double region = m_perlinNoise.Noise(x * 0.0008, z * 0.0008);
	const double mountainMask = MountainMask(region);
	
	const double mountains = m_perlinNoise.Noise(x * 0.015, z * 0.015) * 60.0;


	return static_cast<int>(
		std::lround(
			baseHeight
			+ n
			+ mountainMask * mountains
	));
}


double TerrainGenerator::MountainMask(double region) {
	constexpr double start = 0.10;
	constexpr double end = 0.60;

	double t = (region - start) / (end - start);//start‚©‚ç‚Ç‚ê‚¾‚¯i‚ñ‚Å‚é‚© / start‚©‚ç‘S’·
	t = std::clamp(t, 0.0, 1.0);

	return t * t * (3.0 - 2.0 * t);

}