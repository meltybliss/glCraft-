#pragma once
#include <queue>
#include <unordered_set>
#include "LightNode.h"


enum class LightType {
	SKY = 0,
	BLOCK
};

enum class Phase {
	REMOVE,
	ADD
};


struct RemoveNode {

	int64_t worldX;
	int64_t worldY;
	int64_t worldZ;

	uint8_t oldLight;

};

struct LightTask {

	LightType lightType;

	std::queue<LightNode> bfs_queue;
	std::queue<RemoveNode> remove_queue;
	std::unordered_set<uint64_t> touchedChunkKeys;

	Phase phase = Phase::ADD;
	uint8_t emissionAfterRemove = 0;//if greater than 0, task of propagating block emission will be created

};