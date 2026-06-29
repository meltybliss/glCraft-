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


void LightEngine::AddSkyLightLevel(
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

	uint8_t oldLevel = c->GetSkyLight(lx, ly, lz);
	if (oldLevel >= level) return;


	c->SetSkyLights(lx, ly, lz, level);
	task.bfs_queue.push({ worldX, worldY, worldZ, level });


	const int32_t x = floorDiv(worldX, Chunk::CHUNK_WIDTH);
	const int32_t z = floorDiv(worldZ, Chunk::CHUNK_DEPTH);

	task.touchedChunkKeys.insert(Index(x, z));
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

			if (ny >= Chunk::CHUNK_HEIGHT || ny < 0) continue;


			lightLevel--;

			if (dir[1] == -1 && oldLightLevel == 15) {
				lightLevel = oldLightLevel;
			}
			
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



void LightEngine::StartRemoveBlockLightTask(
	World& w,
	int64_t worldX,
	int64_t worldY,
	int64_t worldZ,
	LightTask& task

) {

	if (worldY < 0 || worldY >= Chunk::CHUNK_HEIGHT) return;

	const uint8_t oldLight = w.GetBlockLightGlobal(
		worldX,
		worldY,
		worldZ

	);

	if (oldLight == 0) return;

	w.SetBlockLightGlobal(worldX, worldY, worldZ, 0);

	task.remove_queue.push({
		worldX,
		worldY,
		worldZ,
		oldLight

	});

	task.lightType = LightType::BLOCK;
	task.phase = Phase::REMOVE;
}


void LightEngine::StartRemoveSkyLightTask(
	World& w,
	int64_t worldX,
	int64_t worldY,
	int64_t worldZ,
	LightTask& task
) {

	if (worldY < 0 || worldY >= Chunk::CHUNK_HEIGHT) return;

	const uint8_t oldLight = w.GetSkyLightGlobal(
		worldX,
		worldY,
		worldZ

	);

	if (oldLight == 0) return;

	w.SetSkyLightGlobal(worldX, worldY, worldZ, 0);

	task.remove_queue.push({
		worldX,
		worldY,
		worldZ,
		oldLight

	});

	task.lightType = LightType::SKY;
	task.phase = Phase::REMOVE;


}



bool LightEngine::Propagate_RemoveSkylight(
	World& w,
	LightTask& task,
	const int taskBudget
) {

	int budget = taskBudget;
	auto& bfs = task.remove_queue;


	static constexpr int dirs[6][3] = {
		{ 1, 0, 0 },
		{-1, 0, 0 },
		{ 0, 1, 0 },
		{ 0,-1, 0 },
		{ 0, 0, 1 },
		{ 0, 0,-1 }
	};

	while (!bfs.empty() && budget > 0) {
		budget--;

		const RemoveNode removeNode = bfs.front();
		bfs.pop();

		for (const auto& dir : dirs) {
			const int64_t nx = removeNode.worldX + dir[0];
			const int64_t ny = removeNode.worldY + dir[1];
			const int64_t nz = removeNode.worldZ + dir[2];

			if (ny < 0 || ny >= Chunk::CHUNK_HEIGHT) continue;

			if (w.GetBlockGlobal(nx, ny, nz) != 0) continue;

			const uint8_t neighborLight =
				w.GetSkyLightGlobal(nx, ny, nz);

			if (neighborLight == 0) continue;

			const bool isDirectSkyBelow =
				dir[0] == 0 &&
				dir[1] == -1 &&
				dir[2] == 0 &&
				removeNode.oldLight == 15 &&
				neighborLight == 15;

			if (isDirectSkyBelow || neighborLight < removeNode.oldLight) {
				w.SetSkyLightGlobal(nx, ny, nz, 0);

				task.remove_queue.push({
					nx,
					ny,
					nz,
					neighborLight
				});

				const int32_t cx = floorDiv(nx, Chunk::CHUNK_WIDTH);
				const int32_t cz = floorDiv(nz, Chunk::CHUNK_DEPTH);

				task.touchedChunkKeys.insert(Index(cx, cz));
			}
			else {
				task.bfs_queue.push({
					nx,
					ny,
					nz,
					neighborLight
				});
			}
		}
	}


	if (!task.remove_queue.empty()) {
		return false;
	}

	task.phase = Phase::ADD;
	return true;
}


bool LightEngine::Propagate_RemoveBlockLight(
	World& w,
	LightTask& task,
	const int taskBudget
) {

	int budget = taskBudget;
	auto& bfs = task.remove_queue;

	static constexpr int dirs[6][3] = {
		{ 1, 0, 0 },
		{-1, 0, 0 },
		{ 0, 1, 0 },
		{ 0,-1, 0 },
		{ 0, 0, 1 },
		{ 0, 0,-1 }
	};


	while (!bfs.empty() && budget > 0) {
		budget--;

		const RemoveNode removeNode = bfs.front();
		bfs.pop();

		for (const auto& dir : dirs) {
			const int64_t nx = removeNode.worldX + dir[0];
			const int64_t ny = removeNode.worldY + dir[1];
			const int64_t nz = removeNode.worldZ + dir[2];

			if (ny < 0 || ny >= Chunk::CHUNK_HEIGHT) continue;

			if (w.GetBlockGlobal(nx, ny, nz) != 0) continue;

			const uint8_t neighborLight =
				w.GetBlockLightGlobal(nx, ny, nz);

			if (neighborLight == 0) continue;

			if (neighborLight < removeNode.oldLight) {
				w.SetBlockLightGlobal(nx, ny, nz, 0);

				task.remove_queue.push({
					nx,
					ny,
					nz,
					neighborLight
				});

				int32_t cx = floorDiv(nx, Chunk::CHUNK_WIDTH);
				int32_t cz = floorDiv(nz, Chunk::CHUNK_DEPTH);

				task.touchedChunkKeys.insert(Index(cx, cz));

			}
			else {
				

				task.bfs_queue.push({
					nx,
					ny,
					nz,
					neighborLight
				});

			}

		}
	}


	if (!task.remove_queue.empty()) {
		return false;
	}

	task.phase = Phase::ADD;
	return true;

}