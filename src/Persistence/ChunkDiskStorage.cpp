#include "Persistence/ChunkDiskStorage.h"
#include "World/Chunk.h"


bool ChunkDiskStorage::SaveToDisk(ChunkSaveTask& task) const {

	ChunkSaveData& saveData = task.saveData;

	std::filesystem::create_directories(chunksPath);

	std::filesystem::path filePath =
		chunksPath /
		(
			"c_" +
			std::to_string(saveData.coord.x) +
			"_" +
			std::to_string(saveData.coord.z) +
			".bin"
		);


	std::ofstream file(filePath, std::ios::binary);
	if (!file) return false;

	constexpr char magic[4] = {
		'G', 'L', 'C', 'K'
	};


	file.write(magic, sizeof(magic));

	file.write(
		reinterpret_cast<const char*>(&saveData.coord.x),
		sizeof(saveData.coord.x)
	);

	file.write(
		reinterpret_cast<const char*>(&saveData.coord.z),
		sizeof(saveData.coord.z)
	);


	file.write(
		reinterpret_cast<const char*>(saveData.blocks.data()),
		sizeof(BlockType) * saveData.blocks.size()
	);


	return file.good();
}


ChunkDiskLoadResult ChunkDiskStorage::LoadFromDisk(const ChunkLoadTask& task) {

    const auto path = GetChunkPath(task.coord);

	 if (!std::filesystem::exists(path))
    {
        return {
            .status = ChunkLoadStatus::NotFound,
            .data = std::nullopt
        };
    }

    std::ifstream file(path, std::ios::binary);

    if (!file)
    {
        return {
            .status = ChunkLoadStatus::IOError,
            .data = std::nullopt
        };
    }

    ChunkSaveData data{};

    constexpr char expectedMagic[4] = {
        'G', 'L', 'C', 'K'
    };

    char loadedMagic[4]{};

    file.read(loadedMagic, sizeof(loadedMagic));

    if (!file)
    {
        return {
            .status = ChunkLoadStatus::Corrupted,
            .data = std::nullopt
        };
    }

    if (std::memcmp(
            expectedMagic,
            loadedMagic,
            sizeof(expectedMagic)
        ) != 0)
    {
        return {
            .status = ChunkLoadStatus::Corrupted,
            .data = std::nullopt
        };
    }

    constexpr std::uintmax_t oldFileSize =
        sizeof(expectedMagic) + sizeof(int32_t) * 2 +
        sizeof(BlockType) * Chunk::CHUNK_SIZE;

    if (std::filesystem::file_size(path) == oldFileSize) {
        int32_t oldCx = 0;
        int32_t oldCz = 0;
        file.read(reinterpret_cast<char*>(&oldCx), sizeof(oldCx));
        file.read(reinterpret_cast<char*>(&oldCz), sizeof(oldCz));
        data.coord = {oldCx, oldCz};
    }
    else {
        file.read(reinterpret_cast<char*>(&data.coord.x), sizeof(data.coord.x));
        file.read(reinterpret_cast<char*>(&data.coord.z), sizeof(data.coord.z));
    }

    if (!file)
    {
        return {
            .status = ChunkLoadStatus::Corrupted,
            .data = std::nullopt
        };
    }

    if (data.coord != task.coord)
    {
        return {
            .status = ChunkLoadStatus::Corrupted,
            .data = std::nullopt
        };
    }

    constexpr uint32_t expectedBlockCount =
        Chunk::CHUNK_SIZE;

    data.blocks.resize(expectedBlockCount);

    file.read(
        reinterpret_cast<char*>(data.blocks.data()),
        static_cast<std::streamsize>(
            sizeof(BlockType) * data.blocks.size()
        )
    );

    if (!file)
    {
        return {
            .status = ChunkLoadStatus::Corrupted,
            .data = std::nullopt
        };
    }

    return {
        .status = ChunkLoadStatus::Loaded,
        .data = std::move(data)
    };
}



bool ChunkDiskStorage::CheckDataExistence(ChunkCoord coord) const {
	return std::filesystem::exists(GetChunkPath(coord));


}
