#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

struct MeshGenerationState {
	std::atomic<std::uint64_t> latest{0};
};

struct MeshGenerationStamp {
	std::shared_ptr<MeshGenerationState> state;
	std::uint64_t value = 0;

	[[nodiscard]] bool IsCurrent() const noexcept {
		return state &&
			state->latest.load(std::memory_order_acquire) == value;
	}
};
