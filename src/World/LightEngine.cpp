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

	uint8_t oldLevel = c->GetLight(lx, ly, lz);
	if (oldLevel >= level) return;

	c->SetLight(lx, ly, lz, level);

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
