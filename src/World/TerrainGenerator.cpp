#include "World/TerrainGenerator.h"
#include <algorithm>
#include <iostream>

void TerrainGenerator::GenerateTerrain(Chunk& c) {

	switch (VERSION) {

		case 1: { 
			GenerateTerrain_V1(c); 
		
			break;
		}
		default: break;

	}


}



void TerrainGenerator::GenerateTerrain_V1(Chunk& c) {


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

				c.SetBlockForGenerator(x, y, z, b);//‚±‚±‚Ì“_‚ÅworldThread‚É‚Íchunk‚Í‚È‚¢‚Ì‚Åthread safe‚Å‚·
			}
		}
	}

}


int TerrainGenerator::GetHeight(int64_t worldX, int64_t worldZ) const {

	double height = m_perlinNoise.GetHeight(worldX, worldZ);
	int h = static_cast<int>(
		std::lround(height)
		);


	return h;
}


double TerrainGenerator::MountainMask(double region) {
	constexpr double start = 0.10;
	constexpr double end = 0.60;

	double t = (region - start) / (end - start);//start‚©‚ç‚Ç‚ê‚¾‚¯i‚ñ‚Å‚é‚© / start‚©‚ç‘S’·
	t = std::clamp(t, 0.0, 1.0);

	return t * t * (3.0 - 2.0 * t);

}