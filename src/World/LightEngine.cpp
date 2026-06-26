#include "World/LightEngine.h"
#include "World/Chunk.h"
#include "World/ChunkUtil.h"
#include "World/World.h"
#include <queue>
#include <unordered_set>

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

	uint8_t oldLevel = c->GetBlockLight(lx, ly, lz);
	if (oldLevel >= level) return;

	c->SetBlockLight(lx, ly, lz, level);

	//
	
	std::unordered_set<uint64_t> touchedChunkKey;

	std::queue<LightNode> bfs_queue;
	bfs_queue.push({worldX, worldY, worldZ, level});

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
		if (baseNode.lightLevel <= 1) {
			continue;
		}

		uint8_t targetLevel = baseNode.lightLevel - 1;
		int64_t x = baseNode.x;
		int64_t y = baseNode.y;
		int64_t z = baseNode.z;


		for (const auto& dir : dirs) {
			int64_t nx = x + dir[0];
			int64_t ny = y + dir[1];
			int64_t nz = z + dir[2];

			if (ny >= Chunk::CHUNK_HEIGHT ||
				ny < 0) {

				continue;
			}

			if (w.GetBlockGlobal(nx, ny, nz) == 0) {

				if (w.GetBlockLightGlobal(nx, ny, nz) < targetLevel) {
					bool ok = w.SetBlockLightGlobal(nx, ny, nz, targetLevel);
					if (ok) {
						int32_t cx = floorDiv(nx, Chunk::CHUNK_WIDTH);
						int32_t cz = floorDiv(nz, Chunk::CHUNK_DEPTH);

						uint64_t key = Index(cx, cz);
						touchedChunkKey.insert(key);

						bfs_queue.push({ nx, ny, nz, targetLevel });
					}
				}
			}
		}

	}


	for (const auto& key : touchedChunkKey) {
		auto c = w.GetTargetChunkFromKey(key);

		c->dirty = true;
		c->urgentUpdateMesh = true;
	}

	touchedChunkKey.clear();

}


void LightEngine::InitializeSkylightForChunk(Chunk& c) {

	//AIRÇ∂Ç·Ç»Ç≠Ç»ÇÈÇ‹Ç≈ÇªÇÃè„Ç©ÇÁÇÃAIRÇÃÇ∆Ç±Ç15light levelÇ…ÇµÇƒÇ¢Ç≠

	c.skyLights.fill(0);

	for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x) {
		for (int z = 0; z < Chunk::CHUNK_DEPTH; ++z) {

			for (int y = Chunk::CHUNK_HEIGHT - 1; y >= 0; --y) {
				

				if (c.GetBlock(x, y, z) != 0) {
					break;
				}

				
				c.SetSkyLights(x, y, z, 15);
			}

		}
	}


}


void LightEngine::Propagate_SkyLight(
	World& w,
	Chunk& c
) {

	int64_t wx = static_cast<int64_t>(c.cx) * Chunk::CHUNK_WIDTH;
	int64_t wz = static_cast<int64_t>(c.cz) * Chunk::CHUNK_DEPTH;

	std::queue<LightNode> bfs_queue;

	std::unordered_set<uint64_t> touchedChunkKeys;

	for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x) {
			for (int z = 0; z < Chunk::CHUNK_DEPTH; ++z) {

				if (c.GetSkyLight(x, y, z) == 15) {
					bfs_queue.push({ wx + x, y, wz + z, 15 });
				}

			}
		}
	}

	constexpr int dirs[6][3] = {
		{1, 0, 0},
		{-1, 0, 0},
		{0, 1, 0},
		{0, -1, 0},
		{0, 0, 1},
		{0, 0, -1}
	};

	while (!bfs_queue.empty()) {
		LightNode targetNode = bfs_queue.front();
		uint8_t oldLightLevel = targetNode.lightLevel;
		
		bfs_queue.pop();

		if (oldLightLevel <= 1) {
			continue;
		}

		for (const auto& dir : dirs) {
			uint8_t lightLevel = targetNode.lightLevel;

			int64_t nx = targetNode.x + dir[0];
			int64_t ny = targetNode.y + dir[1];
			int64_t nz = targetNode.z + dir[2];

			if (lightLevel < 15 && dir[1] != -1) {
				lightLevel--;
			}
			

			if (w.GetBlockGlobal(nx, ny, nz) == 0) {
				if (w.GetSkyLightGlobal(nx, ny, nz) < lightLevel) {
					bool ok = w.SetSkyLightGlobal(nx, ny, nz, lightLevel);

					if (ok) {
						bfs_queue.push({ nx, ny, nz, lightLevel });

						int32_t cx = floorDiv(nx, Chunk::CHUNK_WIDTH);
						int32_t cz = floorDiv(nz, Chunk::CHUNK_DEPTH);

						uint64_t key = Index(cx, cz);

						touchedChunkKeys.insert(key);
					}
				}
			}

		}

	}

	for (const auto& key : touchedChunkKeys) {
		auto* c = w.GetTargetChunkFromKey(key);

		c->dirty = true;

	}

}
