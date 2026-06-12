#pragma once
#include "World/Chunk.h"
#include <unordered_map>
#include <stdint.h>

class World {
public:




private:


	uint64_t Index(int32_t cx, int32_t cz) {
		return (static_cast<uint64_t>(cx) << 32) | cz;
	}
};