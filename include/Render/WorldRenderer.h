#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Render/ChunkMesh.h"
#include "World/PointLight.h"
#include "Shader.h"
#include "Camera.h"
#include <unordered_map>
#include <stdint.h>
#include <optional>
#include "Texture.h"
#include "LightVolumeSnapshot.h"
#include "Core/SnapshotExchanger.h"

class WorldThread;
class World;

class WorldRenderer {
public:
	//void RebuildDrityChunkMesh(World& w);

	explicit WorldRenderer(SnapshotExchanger& exchanger) : m_exchanger(exchanger) {}


	void UploadPendingMeshData(WorldThread& wt);
	void DeleteMeshes(WorldThread& wt);
	void RenderWorld(const Camera& cam, World* w, const PointLightsSnapshot& snapshot);
	void RenderSky(const Camera& cam);


	void RenderShadowPass(const Camera& cam);

	void BeginHDRScene();
	void EndHDRScene();

	void InitSkyShaderAndVAO();

	void InitBaseShader();

	void InitShadownMap();

	void InitHDRFrameBuffer();
	void InitBloom();
	void InitLightVolumeTexture();


	void UpdateDayNightSnap();


	void UpdateLightVolume(const LightVolumeSnapshot& snapshot);

	DayNightSnapshot const GetDayNightSnapForDebug();
private:

	void UploadPointLights(
		Shader& shader,
		const std::vector<PointLight>& lights,
		size_t count,
		const Camera& cam
	);


	void ExtractBrightPixels();
	unsigned int BlurBloom(int passCount);
	void RenderFinalPost(unsigned int bloomTexture);

	void DrawFullscreenTriangle();

private:

	SnapshotExchanger& m_exchanger;
	DayNightSnapshot m_dayNightSnap;


	std::unordered_map<uint64_t, ChunkMesh> m_chunkMeshes;


	std::optional<Shader> baseShader;


	std::unique_ptr<Texture> blockAtlas;
	
	std::mutex dayNightSnapMutex;


	unsigned int m_shadowFBO = 0;
	unsigned int m_shadowDepthTexture = 0;
	std::optional<Shader> m_shadowShader;

	unsigned int m_hdrFBO = 0;
	unsigned int m_sceneTexture = 0;
	unsigned int m_depthRBO = 0;


	unsigned int m_brightFBO = 0;
	unsigned int m_brightTexture = 0;
	unsigned int m_pingPongFBO[2] = { 0, 0 };
	unsigned int m_pingPongTextures[2] = { 0, 0 };

	unsigned int m_lightVolumeTexture = 0;
	

	glm::i64vec3 m_lightVolumeOrigin{ 0 };


	unsigned int m_skyVAO = 0;
	std::optional<Shader> m_skyShader;


	unsigned int m_postVAO = 0;
	std::optional<Shader> m_brightShader;
	std::optional<Shader> m_blurShader;
	std::optional<Shader> m_postShader;



	static constexpr int SHADOW_WIDTH = 2048;
	static constexpr int SHADOW_HEIGHT = 2048;


	glm::mat4 m_lightSpaceMatrix{ 1.0f };
};