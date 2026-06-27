#pragma once
#include <queue>
#include <unordered_set>
#include "LightNode.h"


enum class LightType {
	SKY = 0,
	BLOCK
};

struct LightTask {

	LightType lightType;

	std::queue<LightNode> bfs_queue;
	std::unordered_set<uint64_t> touchedChunkKeys;
};