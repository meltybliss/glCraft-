#pragma once

#include <stdint.h>
#include "World/WorldPos.h"

struct WorldSaveData {

	WorldPos playerPos;

	uint64_t seed;
	double worldTime;//0.0~1.0
	uint32_t generatorVersion;

};