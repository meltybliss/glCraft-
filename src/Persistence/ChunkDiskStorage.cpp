#include "Persistence/ChunkDiskStorage.h"
#include "World/Chunk.h"


bool ChunkDiskStorage::SaveToDisk(ChunkSaveTask& task) const {

	ChunkSaveData& saveData = task.saveData;

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


ChunkDiskLoadResult ChunkDiskStorage::LoadFromDisk(const ChunkLoadTask& task) {

    const auto path = GetChunkPath(task.cx, task.cz);

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

    file.read(
        reinterpret_cast<char*>(&data.cx),
        sizeof(data.cx)
    );

    file.read(
        reinterpret_cast<char*>(&data.cz),
        sizeof(data.cz)
    );

    if (!file)
    {
        return {
            .status = ChunkLoadStatus::Corrupted,
            .data = std::nullopt
        };
    }

    if (data.cx != task.cx || data.cz != task.cz)
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



bool ChunkDiskStorage::CheckDataExistence(int32_t cx, int32_t cz) const {

	std::filesystem::path filePath =
		chunksPath /
		(
			"c_" +
			std::to_string(cx) +
			"_" +
			std::to_string(cz) +
			".bin"
			);


	return std::filesystem::exists(filePath);


}