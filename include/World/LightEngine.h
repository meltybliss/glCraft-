#pragma once
#include "ChunkUtil.h"
#include <stdint.h>

using namespace ChunkUtil;

struct LightNode {
	int64_t x = 0;
	int64_t y = 0;
	int64_t z = 0;
	uint8_t lightLevel = 0;
};


class World;
class LightEngine {
public:

	static void AddLightLevel(
		World& w,
		int64_t worldX,
		int64_t worldY,
		int64_t worldZ,
		uint8_t level
	);

	static void InitializeSkylightForChunk(Chunk& c);

	static void Propagate_SkyLight(
		World& w,
		Chunk& c
	);

private:


};