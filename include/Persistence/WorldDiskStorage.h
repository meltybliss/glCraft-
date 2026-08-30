#pragma once
#include <filesystem>
#include <fstream>
#include <optional>
#include <cstring>

#include "WorldSaveData.h"

class WorldDiskStorage {
public:

	bool SaveToDisk(const WorldSaveData& saveData) const;
	std::optional<WorldSaveData> LoadFromDisk() const;
	bool DeleteWorldFromDisk() const;


private:

	std::filesystem::path worldPath = "worlds/MyWorld";


};
