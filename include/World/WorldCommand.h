#pragma once

#include <cstdint>
#include "World/Chunk.h"

enum class WorldCommandType {
    EDIT_BLOCK
};

struct WorldCommand {
    WorldCommandType type = WorldCommandType::EDIT_BLOCK;

    int64_t worldX = 0;
    int64_t worldY = 0;
    int64_t worldZ = 0;

    BlockType newBlock = BlockType::AIR;
};