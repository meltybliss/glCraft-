#pragma once
#include "ChunkUtil.h"
#include "LightNode.h"
#include "LightTask.h"
#include <stdint.h>

using namespace ChunkUtil;


class World;
class Chunk;
class LightEngine {
public:

	static void AddLightLevel(
		World& w,
		int64_t worldX,
		int64_t worldY,
		int64_t worldZ,
		uint8_t level,
		LightTask& task
	);

	static void Propagate_BlockLight(
		World& w,
		LightTask& task,
		const int taskBudget
	);

	static void InitializeSkylightForChunk(Chunk& c);

	static void Propagate_SkyLight(
		World& w,
		LightTask& task,
		const int taskBudget
	);


	static void CreateSkylightLeakSeeds(Chunk& c, LightTask& task);

private:


};