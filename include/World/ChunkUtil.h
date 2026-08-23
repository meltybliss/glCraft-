#pragma once
#include <stdint.h>
#include "World/ChunkCoord.h"

namespace ChunkUtil {
	[[nodiscard]] constexpr ChunkCoord Index(int64_t cx, int64_t cz) noexcept {
		return { cx, cz };
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
