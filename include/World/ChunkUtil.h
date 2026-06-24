#pragma once
#include <stdint.h>

namespace ChunkUtil {
	inline uint64_t Index(int32_t cx, int32_t cz) {
		uint64_t x = static_cast<uint32_t>(cx);
		uint64_t z = static_cast<uint32_t>(cz);

		return (x << 32) | z;

	}


	inline int64_t floorDiv(int64_t a, int64_t b) {
		int64_t q = a / b;
		int64_t r = a % b;

		if (r != 0 && ((r < 0) != (b < 0))) {
			--q;
		}

		return q;

	}

	inline int floorMod(int64_t a, int b) {//-17, 16 -> 15
		int64_t r = a % b;
		if (r < 0) {
			r += b;
		}

		return static_cast<int>(r);
	}

}