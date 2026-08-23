#pragma once
#include <stdint.h>
#include <vector>
#include "World/BlockData.h"
#include "World/ChunkCoord.h"


struct ChunkSaveData {


	ChunkCoord coord;

	std::vector<BlockType> blocks;




};

enum class ChunkLoadStatus
{
    Loaded,
    NotFound,
    Corrupted,
    IOError
};

struct ChunkDiskLoadResult
{
    ChunkLoadStatus status;
    std::optional<ChunkSaveData> data;
};
