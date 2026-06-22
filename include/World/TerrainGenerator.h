#pragma once
#include "Chunk.h"
#include "Math/PerlinNoise2D.h"

class TerrainGenerator {
public:
	explicit TerrainGenerator(uint64_t seed) : m_perlinNoise(seed) {}

	void GenerateTerrain(Chunk& c);

private:
	PerlinNoise2D m_perlinNoise;

private:
	int GetHeight(int64_t worldX, int64_t worldZ) {
		double n = m_perlinNoise.Noise(
			worldX * 0.01f,
			worldZ * 0.01f
		);

		return 64 + static_cast<int>(n * 18.0f);
	}
};