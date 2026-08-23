#pragma once
#include <array>
#include <memory>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Snapshot/ChunkRenderabilitySnapshot.h"
#include "Render/Texture1D.h"
#include "World/WorldPos.h"

class FogSystem {
public:

	FogSystem();

	void InitRenderableDistTexture();

	void UpdateFog(
		const WorldPos& camera,
		const ChunkRenderabilitySnapshot& snap
	);


	Texture1D* GetRenderableDistTexture() const { return m_renderableDistTexture.get(); }

	void Bind(unsigned int unit) const;

	
private:

	double FindRenderableDistance(const WorldPos& camera, const glm::dvec2& dir, const ChunkRenderabilitySnapshot& snap);

	std::array<float, 360> m_renderableDistances;
	std::unique_ptr<Texture1D> m_renderableDistTexture;

};