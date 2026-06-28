#include "World/LightEngine.h"
#include "World/Chunk.h"
#include "World/ChunkUtil.h"
#include "World/World.h"
#include <queue>
#include <unordered_set>
#include <iostream>

void LightEngine::AddLightLevel(
	World& w,
	int64_t worldX,
	int64_t worldY,
	int64_t worldZ,
	uint8_t level,
	LightTask& task
) {

	if (worldY < 0 || worldY >= Chunk::CHUNK_HEIGHT) {
		return;
	}
	

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
	task.bfs_queue.push({ worldX, worldY, worldZ, level });

}


void LightEngine::Propagate_BlockLight(
	World& w,
	LightTask& task,
	const int taskBudget
) {

	//

	int curBudget = taskBudget;

	auto& touchedChunkKey = task.touchedChunkKeys;
	auto& bfs_queue = task.bfs_queue;



	constexpr int dirs[6][3] = {
		{1, 0, 0},
		{-1, 0, 0},
		{0, 1, 0},
		{0, -1, 0},
		{0, 0, 1},
		{0, 0, -1}
	};

	while (!bfs_queue.empty() && curBudget > 0) {
		LightNode baseNode = bfs_queue.front();

		bfs_queue.pop();

		curBudget--;

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


}


void LightEngine::InitializeSkylightForChunk(Chunk& c) {

	//AIR‚¶‚á‚È‚­‚È‚é‚Ü‚Å‚»‚Ìã‚©‚ç‚ÌAIR‚Ì‚Æ‚±‚ð15light level‚É‚µ‚Ä‚¢‚­

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


void LightEngine::CreateSkylightLeakSeeds(Chunk& c, LightTask& task) {


	auto& bfs_queue = task.bfs_queue;

	int64_t wx = static_cast<int64_t>(c.cx) * Chunk::CHUNK_WIDTH;
	int64_t wz = static_cast<int64_t>(c.cz) * Chunk::CHUNK_DEPTH;


	constexpr int dirs[4][2] = {//x, z
		{1, 0},
		{-1, 0},
		{0, 1},
		{0, -1}

	};

	for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x) {
		for (int z = 0; z < Chunk::CHUNK_DEPTH; ++z) {
			for (int y = Chunk::CHUNK_HEIGHT - 1; y >= 0; --y) {

				if (c.GetSkyLight(x, y, z) != 15) {
					continue;
				}

				for (const auto& dir : dirs) {
					int nx = x + dir[0];
					int nz = z + dir[1];
					
					// ‚Ü‚¸‚Íƒ`ƒƒƒ“ƒN“à‚¾‚¯ˆµ‚¤
					if (nx < 0 || nx >= Chunk::CHUNK_WIDTH ||
						nz < 0 || nz >= Chunk::CHUNK_DEPTH) {
						continue;
					}

					if (c.GetBlock(nx, y, nz) != 0) continue;

					if (c.GetSkyLight(nx, y, nz) >= 14) continue;

					c.SetSkyLights(nx, y, nz, 14);

					bfs_queue.push({
						wx + nx,
						y,
						wz + nz,
						14

					});

				}
			}

		}
	}


}


void LightEngine::Propagate_SkyLight(
	World& w,
	LightTask& task,
	const int taskBudget
) {

	int curBudget = taskBudget;

	auto& bfs_queue = task.bfs_queue;
	auto& touchedChunkKeys = task.touchedChunkKeys;


	constexpr int dirs[6][3] = {
		{1, 0, 0},
		{-1, 0, 0},
		{0, 1, 0},
		{0, -1, 0},
		{0, 0, 1},
		{0, 0, -1}
	};


	while (!bfs_queue.empty() && curBudget > 0) {
		LightNode targetNode = bfs_queue.front();
		uint8_t oldLightLevel = targetNode.lightLevel;
		
		bfs_queue.pop();



		curBudget--;


		if (oldLightLevel <= 1) {
			continue;
		}

		for (const auto& dir : dirs) {
			uint8_t lightLevel = targetNode.lightLevel;

			int64_t nx = targetNode.x + dir[0];
			int64_t ny = targetNode.y + dir[1];
			int64_t nz = targetNode.z + dir[2];

			if (ny >= Chunk::CHUNK_HEIGHT - 1 || ny < 0) continue;


			lightLevel--;
			
			unsigned int b = w.GetBlockGlobal(nx, ny, nz);
			uint8_t s = w.GetSkyLightGlobal(nx, ny, nz);



			if (b == 0) {
				if (s < lightLevel) {
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


}
