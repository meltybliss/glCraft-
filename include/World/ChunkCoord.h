#pragma once
#include <stdint.h>
#include <cstddef>
#include <functional>
#include <compare>

struct ChunkCoord {
    int64_t x = 0;
    int64_t z = 0;

    bool operator==(const ChunkCoord&) const = default;
    auto operator<=>(const ChunkCoord&) const = default;
};


struct ChunkCoordHash {
    std::size_t operator()(const ChunkCoord& c) const noexcept
    {
        std::size_t h1 = std::hash<int64_t>{}(c.x);
        std::size_t h2 = std::hash<int64_t>{}(c.z);

        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL
            + (h1 << 6)
            + (h1 >> 2));
    }
};
