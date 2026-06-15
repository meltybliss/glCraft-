#include "Render/MeshBuilder.h"

MeshData MeshBuilder::BuildChunkMesh(const World& w, const Chunk& c) {
	MeshData result;
	auto& v = result.vertices;
	auto& indices = result.indices;

	const auto& cx = c.cx;
	const auto& cz = c.cz;

	const auto& s = BLOCK_SIZE;

	auto CheckNeighborAir = [&](int nx, int ny, int nz) -> bool {
		if (ny < 0 || ny >= Chunk::CHUNK_HEIGHT) {
			return true;
		}

		if (c.InBounds(nx, ny, nz)) {
			return c.GetBlock(nx, ny, nz) == 0;
		}

		int64_t gx = static_cast<int64_t>(cx) * Chunk::CHUNK_WIDTH + nx;
		int64_t gy = ny;
		int64_t gz = static_cast<int64_t>(cz) * Chunk::CHUNK_DEPTH + nz;

		return w.GetBlockGlobal(gx, gy, gz) == 0;
	};

	for (int y = 0; y < Chunk::CHUNK_HEIGHT; y++) {
		for (int x = 0; x < Chunk::CHUNK_WIDTH; x++) {
			for (int z = 0; z < Chunk::CHUNK_DEPTH; z++) {


				unsigned int b = c.GetBlock(x, y, z);

				if (b == 0) {//AIR
					continue;
				}

				//0----3
				//|	   |
				//1----2
				if (CheckNeighborAir(x - 1, y, z)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x, (float)y,     (float)z + s },
						{ (float)x, (float)y,     (float)z     },
						{ (float)x, (float)y + s, (float)z     },
						{ (float)x, (float)y + s, (float)z + s }
					} };

					AddFace(pointsSet, (BlockType)b, BlockFace::LEFT, indices, v);
				}
				if (CheckNeighborAir(x + 1, y, z)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x + s, (float)y,     (float)z     },
						{ (float)x + s, (float)y,     (float)z + s },
						{ (float)x + s, (float)y + s, (float)z + s },
						{ (float)x + s, (float)y + s, (float)z     }
					} };

					AddFace(pointsSet, (BlockType)b, BlockFace::RIGHT, indices, v);
				}

				if (CheckNeighborAir(x, y - 1, z)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x,     (float)y, (float)z     },
						{ (float)x + s, (float)y, (float)z     },
						{ (float)x + s, (float)y, (float)z + s },
						{ (float)x,     (float)y, (float)z + s }
					} };



					AddFace(pointsSet, (BlockType)b, BlockFace::BOTTOM, indices, v);

				}

				if (CheckNeighborAir(x, y + 1, z)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x,     (float)y + s, (float)z + s },
						{ (float)x + s, (float)y + s, (float)z + s },
						{ (float)x + s, (float)y + s, (float)z     },
						{ (float)x,     (float)y + s, (float)z     }
					} };


					AddFace(pointsSet, (BlockType)b, BlockFace::TOP, indices, v);
				}


				if (CheckNeighborAir(x, y, z - 1)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x + s, (float)y,     (float)z },
						{ (float)x,     (float)y,     (float)z },
						{ (float)x,     (float)y + s, (float)z },
						{ (float)x + s, (float)y + s, (float)z }
					} };


					AddFace(pointsSet, (BlockType)b, BlockFace::FRONT, indices, v);
				}

				if (CheckNeighborAir(x, y, z + 1)) {
					std::array<std::array<float, 3>, 4> pointsSet = { {
						{ (float)x,     (float)y,     (float)z + s },
						{ (float)x + s, (float)y,     (float)z + s },
						{ (float)x + s, (float)y + s, (float)z + s },
						{ (float)x,     (float)y + s, (float)z + s }
					} };

					AddFace(pointsSet, (BlockType)b, BlockFace::BACK, indices, v);
				}
				
			}

		}

	}

	return result;


}


UV MeshBuilder::GetUV(const BlockType b, uint8_t index, BlockFace face) {
	if (b == BlockType::AIR) {
		return { 0.f, 0.f };
	}

	return GetBlockFaceUV(b, index, face);

}

UV MeshBuilder::GetBlockFaceUV(const BlockType b, uint8_t index, BlockFace face) {
	switch (b) {
		case BlockType::GRASS: {
			if (face == BlockFace::TOP) {
				return GetTileVertexUV(
					index,
					2,
					0
				);
			}
			if (face == BlockFace::BOTTOM) {
				return GetTileVertexUV(
					index,
					18,
					1
				);
			}

			return GetTileVertexUV(
				index,
				3,
				0
			);
		}

		case BlockType::DIRT: {
			return GetTileVertexUV(index, 18, 1);
		}

		case BlockType::STONE: {
			return GetTileVertexUV(index, 19, 0);
		}

		default: {
			return GetTileVertexUV(index, 0, 0);
		}
	}


}

UV MeshBuilder::GetTileVertexUV(uint8_t index, int tileX, int tileY) {
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

	return { 0.f, 0.f };
}


void MeshBuilder::AddFace(
	std::array<std::array<float, 3>, 4>& pointsSet, 
	const BlockType b, 
	const BlockFace face,
	std::vector<unsigned int>& indices, std::vector<float>& v)
{
	unsigned int base = static_cast<unsigned int>(v.size() / 5);

	//base+0,1,2,3
	for (int i = 0; i < 4; i++) {
		UV uv = GetUV((BlockType)b, i, face);
		const std::array<float, 3>& points = pointsSet[i];

		v.insert(v.end(), { points[0], points[1], points[2], uv.u, uv.v });

	}

	indices.insert(
		indices.end(),
		{base + 1, base + 2, base + 3,
		 base + 0, base + 1, base + 3}
	);

}