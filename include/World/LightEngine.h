#pragma once
#include "ChunkUtil.h"
#include <stdint.h>

using namespace ChunkUtil;

struct LightNode {
	int lx = 0;
	int ly = 0;
	int lz = 0;
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

	static void Propagate_SkyLight(
		World& w,
		int64_t worldX,
		int64_t worldY,
		int64_t worldZ 
	);

private:


};