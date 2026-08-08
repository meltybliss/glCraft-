#pragma once
#include <stdint.h>
#include <vector>
#include "World/BlockData.h"


struct ChunkSaveData {


	int32_t cx;
	int32_t cz;

	std::vector<BlockType> blocks;




};