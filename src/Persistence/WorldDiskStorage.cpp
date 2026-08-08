#include "Persistence/WorldDiskStorage.h"

bool WorldDiskStorage::SaveToDisk(const WorldSaveData& saveData) const {

	std::filesystem::create_directories(worldPath);

	std::filesystem::path filePath = worldPath / "World.bin";

	std::ofstream file(filePath, std::ios::binary);
	if (!file) return false;

	constexpr char magic[4] = {
		'G', 'L', 'W', 'D'
	};

	file.write(magic, sizeof(magic));
	
	file.write(
		reinterpret_cast<const char*>(&saveData.playerPos.block.x),
		sizeof(int64_t)
	);

	file.write(
		reinterpret_cast<const char*>(&saveData.playerPos.block.y),
		sizeof(int64_t)
	);

	file.write(
		reinterpret_cast<const char*>(&saveData.playerPos.block.z),
		sizeof(int64_t)
	);


	file.write(
		reinterpret_cast<const char*>(&saveData.playerPos.local.x),
		sizeof(double)
	);

	file.write(
		reinterpret_cast<const char*>(&saveData.playerPos.local.y),
		sizeof(double)
	);

	file.write(
		reinterpret_cast<const char*>(&saveData.playerPos.local.z),
		sizeof(double)
	);


	file.write(
		reinterpret_cast<const char*>(&saveData.seed),
		sizeof(uint64_t)
	);

	file.write(
		reinterpret_cast<const char*>(&saveData.worldTime),
		sizeof(double)
	);


	file.write(
		reinterpret_cast<const char*>(&saveData.generatorVersion),
		sizeof(uint32_t)
	);



	return file.good();
}



std::optional<WorldSaveData>
WorldDiskStorage::LoadFromDisk() const {


	std::filesystem::path filePath = worldPath / "World.bin";
	std::ifstream file(filePath, std::ios::binary);
	if (!file) return std::nullopt;

	WorldSaveData data{};

	constexpr char expectedMagic[4] = {
		'G', 'L', 'W', 'D'
	};

	char loadedMagic[4]{};

	file.read(loadedMagic, sizeof(expectedMagic));

	if (!file) return std::nullopt;

	if (std::memcmp(
		expectedMagic,
		loadedMagic,
		sizeof(expectedMagic)) != 0) {

		return std::nullopt;

	}


	file.read(
		reinterpret_cast<char*>(&data.playerPos.block.x),
		sizeof(int64_t)
	);

	file.read(
		reinterpret_cast<char*>(&data.playerPos.block.y),
		sizeof(int64_t)
	);

	file.read(
		reinterpret_cast<char*>(&data.playerPos.block.z),
		sizeof(int64_t)
	);

	file.read(
		reinterpret_cast<char*>(&data.playerPos.local.x),
		sizeof(double)
	);

	file.read(
		reinterpret_cast<char*>(&data.playerPos.local.y),
		sizeof(double)
	);

	file.read(
		reinterpret_cast<char*>(&data.playerPos.local.z),
		sizeof(double)
	);


	file.read(
		reinterpret_cast<char*>(&data.seed),
		sizeof(uint64_t)
	);

	file.read(
		reinterpret_cast<char*>(&data.worldTime),
		sizeof(double)
	);


	file.read(
		reinterpret_cast<char*>(&data.generatorVersion),
		sizeof(uint32_t)
	);


	if (!file) return std::nullopt;

	return data;
}