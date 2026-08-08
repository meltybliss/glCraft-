#pragma once

#include <stdint.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <optional>

#include "ChunkSaveData.h"

class ChunkDiskStorage {
public:

	bool SaveToDisk(const ChunkSaveData& saveData) const;

	std::optional<ChunkSaveData> LoadFromDisk(int32_t cx, int32_t cz);

private:

	std::filesystem::path worldPath = "worlds/MyWorld";
	std::filesystem::path chunksPath = worldPath / "chunks";

	std::filesystem::path GetChunkPath(int32_t cx, int32_t cz) const {
		return 
			worldPath /
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