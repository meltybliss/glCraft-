#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <vector>

namespace LIGHT_VOLUME_SIZE {
	constexpr int LIGHT_VOLUME_WIDTH = 64;
	constexpr int LIGHT_VOLUME_HEIGHT = 64;
	constexpr int LIGHT_VOLUME_DEPTH = 64;
}

struct LightVolumeRegion {
	glm::ivec3 offset{0};
	glm::ivec3 size{0};
	std::vector<float> pixels;
};

struct LightVolumeUploadBox {
	glm::ivec3 textureOffset{0};
	glm::ivec3 size{0};
	glm::ivec3 sourceOffset{0};
};

inline int WrapLightVolumeIndex(int value, int size) {
	const int remainder = value % size;
	return remainder < 0 ? remainder + size : remainder;
}

inline std::vector<LightVolumeRegion> BuildLightVolumeMovementRegions(
	const glm::i64vec3& delta
) {
	using namespace LIGHT_VOLUME_SIZE;
	const glm::ivec3 volumeSize{
		LIGHT_VOLUME_WIDTH,
		LIGHT_VOLUME_HEIGHT,
		LIGHT_VOLUME_DEPTH
	};
	const glm::ivec3 d = glm::ivec3(delta);
	std::vector<LightVolumeRegion> regions;

	auto addRegion = [&](const glm::ivec3& offset, const glm::ivec3& size) {
		if (size.x > 0 && size.y > 0 && size.z > 0) {
			regions.push_back({offset, size, {}});
		}
	};

	const glm::ivec3 overlapOffset{
		std::max(-d.x, 0),
		std::max(-d.y, 0),
		std::max(-d.z, 0)
	};
	const glm::ivec3 overlapSize = volumeSize - glm::abs(d);

	if (d.x != 0) {
		addRegion(
			{d.x > 0 ? volumeSize.x - d.x : 0, 0, 0},
			{std::abs(d.x), volumeSize.y, volumeSize.z}
		);
	}
	if (d.y != 0) {
		addRegion(
			{overlapOffset.x, d.y > 0 ? volumeSize.y - d.y : 0, 0},
			{overlapSize.x, std::abs(d.y), volumeSize.z}
		);
	}
	if (d.z != 0) {
		addRegion(
			{overlapOffset.x, overlapOffset.y,
				d.z > 0 ? volumeSize.z - d.z : 0},
			{overlapSize.x, overlapSize.y, std::abs(d.z)}
		);
	}
	return regions;
}

inline std::vector<LightVolumeUploadBox> SplitLightVolumeUpload(
	const glm::ivec3& logicalOffset,
	const glm::ivec3& regionSize,
	const glm::ivec3& ringOffset
) {
	using namespace LIGHT_VOLUME_SIZE;
	const glm::ivec3 volumeSize{
		LIGHT_VOLUME_WIDTH,
		LIGHT_VOLUME_HEIGHT,
		LIGHT_VOLUME_DEPTH
	};
	struct AxisPiece { int texture = 0; int source = 0; int size = 0; };
	std::array<std::array<AxisPiece, 2>, 3> pieces{};
	std::array<int, 3> counts{1, 1, 1};
	for (int axis = 0; axis < 3; ++axis) {
		const int start = WrapLightVolumeIndex(
			logicalOffset[axis] + ringOffset[axis], volumeSize[axis]);
		const int firstSize = std::min(regionSize[axis], volumeSize[axis] - start);
		pieces[axis][0] = {start, 0, firstSize};
		if (firstSize < regionSize[axis]) {
			pieces[axis][1] = {0, firstSize, regionSize[axis] - firstSize};
			counts[axis] = 2;
		}
	}

	std::vector<LightVolumeUploadBox> boxes;
	for (int z = 0; z < counts[2]; ++z) {
		for (int y = 0; y < counts[1]; ++y) {
			for (int x = 0; x < counts[0]; ++x) {
				boxes.push_back({
					{pieces[0][x].texture, pieces[1][y].texture, pieces[2][z].texture},
					{pieces[0][x].size, pieces[1][y].size, pieces[2][z].size},
					{pieces[0][x].source, pieces[1][y].source, pieces[2][z].source}
				});
			}
		}
	}
	return boxes;
}

struct LightVolumeSnapshot {

	std::vector<float> pixels;
	std::vector<LightVolumeRegion> regions;
	glm::i64vec3 origin;
	glm::i64vec3 previousOrigin;
	bool fullUpdate = true;

};


