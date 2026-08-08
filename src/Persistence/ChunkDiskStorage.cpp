#include "Persistence/ChunkDiskStorage.h"
#include "World/Chunk.h"


bool ChunkDiskStorage::SaveToDisk(const ChunkSaveData& saveData) const {

	std::filesystem::create_directories(chunksPath);

	std::filesystem::path filePath =
		chunksPath /
		(
			"c_" +
			std::to_string(saveData.cx) +
			"_" +
			std::to_string(saveData.cz) +
			".bin"
		);


	std::ofstream file(filePath, std::ios::binary);
	if (!file) return false;

	constexpr char magic[4] = {
		'G', 'L', 'C', 'K'
	};


	file.write(magic, sizeof(magic));

	file.write(
		reinterpret_cast<const char*>(&saveData.cx),
		sizeof(saveData.cx)
	);

	file.write(
		reinterpret_cast<const char*>(&saveData.cz),
		sizeof(saveData.cz)
	);


	file.write(
		reinterpret_cast<const char*>(saveData.blocks.data()),
		sizeof(BlockType) * saveData.blocks.size()
	);


	return file.good();
}



std::optional<ChunkSaveData>
ChunkDiskStorage::LoadFromDisk(int32_t cx, int32_t cz) {

	std::ifstream file(GetChunkPath(cx, cz), std::ios::binary);

	if (!file) return std::nullopt;

	ChunkSaveData data;

	constexpr char expectedMagic[4] = {
		'G', 'L', 'C', 'K'
	};

	const int expectedBlockCount = Chunk::CHUNK_SIZE;


	char loadedMagic[4]{};

	file.read(
		&loadedMagic[0],
		sizeof(loadedMagic)
	);

	if (!file) return std::nullopt;

	if (memcmp(
		expectedMagic,
		loadedMagic,
		sizeof(expectedMagic)) != 0) {

		return std::nullopt;

	}


	file.read(
		reinterpret_cast<char*>(&data.cx),
		sizeof(data.cx)
	);


	file.read(
		reinterpret_cast<char*>(&data.cz),
		sizeof(data.cz)
	);


	if (data.cx != cx || data.cz != cz)
		return std::nullopt;


	data.blocks.resize(expectedBlockCount);

	file.read(
		reinterpret_cast<char*>(data.blocks.data()),
		sizeof(BlockType) * expectedBlockCount
	);

	if (!file) return std::nullopt;


	return data;
}