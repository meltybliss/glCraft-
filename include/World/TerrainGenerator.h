#pragma once
#include "Chunk.h"
#include "Math/PerlinNoise2D.h"

class TerrainGenerator {
public:
	//*注意* Generatorは各バージョンの生成計算の仕組みを残しておくこと 
	explicit TerrainGenerator(uint64_t seed) : m_perlinNoise(seed) {}

	void GenerateTerrain(Chunk& c);

	uint32_t GetVersion() const {return VERSION;}
private:
	PerlinNoise2D m_perlinNoise;

	uint32_t VERSION = 1;

private:
	int GetHeight(int64_t worldX, int64_t worldZ) const;

	void GenerateTerrain_V1(Chunk& c);

	static double MountainMask(double region);
};