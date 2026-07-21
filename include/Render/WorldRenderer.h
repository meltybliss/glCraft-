#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Render/ChunkMesh.h"
#include "Shader.h"
#include "Camera.h"
#include <unordered_map>
#include <stdint.h>
#include <optional>

class WorldThread;

class WorldRenderer {
public:
	//void RebuildDrityChunkMesh(World& w);


	void UploadPendingMeshData(WorldThread& wt);
	void DeleteMeshes(WorldThread& wt);
	void RenderWorld(Shader& shader, const Camera& cam);
	void RenderSky();


	void RenderShadowPass(const Camera& cam);



	void InitSkyRender();
	void InitShadownMap();

private:



private:

	std::unordered_map<uint64_t, ChunkMesh> m_chunkMeshes;

	unsigned int m_shadowFBO = 0;
	unsigned int m_shadowDepthTexture = 0;
	std::optional<Shader> m_shadowShader;


	unsigned int m_skyVAO = 0;
	std::optional<Shader> m_skyShader;


	static constexpr int SHADOW_WIDTH = 2048;
	static constexpr int SHADOW_HEIGHT = 2048;

	glm::mat4 m_lightSpaceMatrix{ 1.0f };
};