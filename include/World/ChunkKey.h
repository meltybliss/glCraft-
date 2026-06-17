#pragma once
#include <stdint.h>

namespace ChunkKey {
	inline uint64_t Index(int32_t cx, int32_t cz) {
		uint64_t x = static_cast<uint32_t>(cx);
		uint64_t z = static_cast<uint32_t>(cz);

		return (x << 32) | z;

	}

}