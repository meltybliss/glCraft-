#include "World/LightEngine.h"
#include "World/Chunk.h"
#include "World/World.h"
#include <queue>

void LightEngine::AddLightLevel(
	World& w,
	int64_t worldX,
	int64_t worldY,
	int64_t worldZ,
	uint8_t level
) {

	int32_t cx = floorDiv(worldX, Chunk::CHUNK_WIDTH);
	int32_t cz = floorDiv(worldZ, Chunk::CHUNK_DEPTH);

	Chunk* c = w.GetTargetChunk(cx, cz);
	if (!c) return;

	int lx = floorMod(worldX, Chunk::CHUNK_WIDTH);
	int ly = worldY;
	int lz = floorMod(worldZ, Chunk::CHUNK_DEPTH);

	uint8_t oldLevel = c->GetLight(lx, ly, lz);
	if (oldLevel >= level) return;

	c->SetLight(lx, ly, lz, level);

	//
	std::queue<LightNode> bfs_queue;
	bfs_queue.push({lx, ly, lz, level});

	constexpr int dirs[6][3] = {
		{1, 0, 0},
		{-1, 0, 0},
		{0, 1, 0},
		{0, -1, 0},
		{0, 0, 1},
		{0, 0, -1}
	};

	while (!bfs_queue.empty()) {
		LightNode baseNode = bfs_queue.front();

		bfs_queue.pop();
		if (baseNode.lightLevel <= 0) {
			continue;
		}

		uint8_t targetLevel = baseNode.lightLevel - 1;
		int x = baseNode.lx;
		int y = baseNode.ly;
		int z = baseNode.lz;


		for (const auto& dir : dirs) {
			int nx = x + dir[0];
			int ny = y + dir[1];
			int nz = z + dir[2];

			if (!c->InBounds(nx, ny, nz)) {
				continue;
			}

			if (c->GetBlock(nx, ny, nz) == 0) {

				if (c->GetLight(nx, ny, nz) < targetLevel) {
					c->SetLight(nx, ny, nz, targetLevel);

					bfs_queue.push({ nx, ny, nz, targetLevel });
				}
			}
		}

		/*for (int dx = -1; dx <= 1; ++dx) {
			for (int dy = -1; dy <= 1; ++dy) {
				for (int dz = -1; dz <= 1; ++dz) {
					if (dx == 0 && dy == 0 && dz == 0) continue;

					int nx = x + dx;
					int ny = y + dy;
					int nz = z + dz;

					if (!c->InBounds(nx, ny, nz)) {
						continue;
					}

					if (c->GetBlock(nx, ny, nz) == 0) {

						if (c->GetLight(nx, ny, nz) < targetLevel) {
							c->SetLight(nx, ny, nz, targetLevel);

							bfs_queue.push({nx, ny, nz, targetLevel});
						}
					}


				}
			}
		}*/
	}

}


void LightEngine::Propagate_SkyLight(
	World& w,
	int64_t worldX,
	int64_t worldY,
	int64_t worldZ
) {


	int32_t cx = floorDiv(worldX, Chunk::CHUNK_WIDTH);
	int32_t cz = floorDiv(worldZ, Chunk::CHUNK_DEPTH);

	Chunk* c = w.GetTargetChunk(cx, cz);
	if (!c) return;

	auto& lightMap = c->blockLights;

	std::queue<uint8_t> bfs_queue;



}
