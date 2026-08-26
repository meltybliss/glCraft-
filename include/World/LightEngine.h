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
		const glm::vec3& lightColor,
		LightTask& task
	);

	static void AddSkyLightLevel(
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
		int& taskBudget
	);

	static void InitializeSkylightForChunk(Chunk& c);

	static void Propagate_SkyLight(
		World& w,
		LightTask& task,
		int& taskBudget
	);

	static bool Propagate_RemoveBlockLight(
		World& w,
		LightTask& task,
		int& taskBudget
	);

	static bool Propagate_RemoveSkylight(
		World& w,
		LightTask& task,
		int& taskBudget
	);

	static void CreateSkylightLeakSeeds(Chunk& c, LightTask& task);

	static void StartRemoveBlockLightTask(
		World& w,
		int64_t worldX,
		int64_t worldY,
		int64_t worldZ,
		LightTask& task

	);

	static void StartRemoveSkyLightTask(
		World& w,
		int64_t worldX,
		int64_t worldY,
		int64_t worldZ,
		LightTask& task
	);


	static void InsertGeoDirtyChunks(//光はいじらずブロックだけをいじる場合のmesh dirty
		World& w,
		int64_t worldX,
		int64_t worldY,
		int64_t worldZ,
		LightTask& task);

	static void RecordBlockLightChange(
		LightTask& task,
		int64_t x,
		int64_t y,
		int64_t z);

private:


};