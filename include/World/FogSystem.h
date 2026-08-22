#pragma once
#include <array>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Snapshot/ChunkRenderabilitySnapshot.h"
#include "Render/Texture1D.h"

class FogSystem {
public:

	void InitRenderableDistTexture();

	void UpdateFog(
		const glm::dvec3& camera,
		const ChunkRenderabilitySnapshot& snap
	);


private:

	double FindRenderableDistance(const glm::dvec2& cameraXZ, const glm::dvec2& dir, const ChunkRenderabilitySnapshot& snap);

	std::array<float, 360> m_renderableDistances;
	std::unique_ptr<Texture1D> m_renderableDistTexture;

};