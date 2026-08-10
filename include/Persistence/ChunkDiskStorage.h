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

	bool CheckDataExistence(int32_t cx, int32_t cz) const;

private:

	std::filesystem::path worldPath = "worlds/MyWorld";
	std::filesystem::path chunksPath = worldPath / "chunks";

	std::filesystem::path GetChunkPath(int32_t cx, int32_t cz) const {
		return 
			chunksPath /
			(
				"c_" +
				std::to_string(cx) +
				"_" +
				std::to_string(cz) +
				".bin"
			);

	}

};