#include "Render/MeshBuilder.h"

MeshData MeshBuilder::BuildChunkMesh(const Chunk& c) {
	MeshData result;


	for (int y = 0; y < Chunk::CHUNK_HEIGHT; y++) {
		for (int x = 0; x < Chunk::CHUNK_WIDTH; x++) {
			for (int z = 0; z < Chunk::CHUNK_DEPTH; z++) {


				const auto& b = c.blocks[Chunk::Index(x, y, z)];

				

			}
		}

	}


}


UV MeshBuilder::GetUV(const BlockType b, uint8_t index, BlockFace face) {
	if (b == BlockType::AIR) {
		return { 0.f, 0.f };
	}

	return GetBlockTile(b, index, face);

}

UV MeshBuilder::GetBlockTile(const BlockType b, uint8_t index, BlockFace face) {
	switch (b) {
		case BlockType::GRASS: {
			if (face == BlockFace::TOP) {
				return GetBlockUV(
					index,
					2,
					0
				);
			}
			if (face == BlockFace::BOTTOM) {
				return GetBlockUV(
					index,
					18,
					1
				);
			}

			return GetBlockUV(
				index,
				3,
				0
			);
		}

		case BlockType::DIRT: {
			return GetBlockUV(index, 18, 1);
		}

		case BlockType::STONE: {
			return GetBlockUV(index, 19, 0);
		}
	}


}

UV MeshBuilder::GetBlockUV(uint8_t index, int tileX, int tileY) const {
	UVPoint p = (UVPoint)index;

	float tileUV_X = 1.0f / atlasTilesX;
	float tileUV_Y = 1.0f / atlasTilesY;

	float u0 = tileUV_X * tileX;
	float v0 = tileUV_Y * tileY;

	float u1 = u0 + tileUV_X;
	float v1 = v0 + tileUV_Y;

	if (p == UVPoint::LeftTop) {
		return { u0, v1 };
	}
	if (p == UVPoint::LeftBottom) {
		return { u0, v0 };
	}
	if (p == UVPoint::RightBottom) {
		return { u1, v0 };
	}
	if (p == UVPoint::RightTop) {
		return { u1, v1 };
	}
}