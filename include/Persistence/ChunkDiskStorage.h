#pragma once

#include <stdint.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <optional>
#include <cstring>

#include "ChunkLoadTask.h"
#include "ChunkSaveTask.h"

class ChunkDiskStorage {
public:

	bool SaveToDisk(ChunkSaveTask& task) const;

	ChunkDiskLoadResult LoadFromDisk(const ChunkLoadTask& task);

	bool CheckDataExistence(ChunkCoord coord) const;

private:

	std::filesystem::path worldPath = "worlds/MyWorld";
	std::filesystem::path chunksPath = worldPath / "chunks";

	std::filesystem::path GetChunkPath(ChunkCoord coord) const {
		return 
			chunksPath /
			(
				"c_" +
				std::to_string(coord.x) +
				"_" +
				std::to_string(coord.z) +
				".bin"
			);

	}

};
