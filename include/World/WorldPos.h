#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct WorldPos {

	glm::i64vec3 block{ 0, 0, 0 };
	glm::dvec3 local{ 0.f, 0.f, 0.f };

};


inline void NormalizeAxis(int64_t& block, double& local) {

	if (local >= 0.0 && local < 1.0) return;

	int64_t portion = static_cast<int>(std::floor(local));

	local -= portion;
	block += portion;


}

inline void NormalizePosition(WorldPos& pos) {

	NormalizeAxis(pos.block.x, pos.local.x);
	NormalizeAxis(pos.block.y, pos.local.y);
	NormalizeAxis(pos.block.z, pos.local.z);

}


inline glm::dvec3 GetRelativePos(const WorldPos& origin, const WorldPos& target) {

	const glm::i64vec3 blockDelta =
		target.block - origin.block;

	return glm::dvec3(blockDelta)
		+ target.local - origin.local;

}