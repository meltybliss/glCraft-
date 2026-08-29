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
#include "ImageTexture2D.h"
#include "RenderTexture2D.h"
#include "DepthTexture2D.h"
#include "DataTexture3D.h"
#include "SkyRenderingConfig.h"
#include "World/FogSystem.h"
#include "World/ChunkCoord.h"
#include "Snapshot/SnapshotExchanger.h"
#include "Snapshot/ChunkRenderabilitySnapshot.h"


using namespace SkyRenderingConfig;


class WorldThread;
class World;

class WorldRenderer {
public:
	//void RebuildDrityChunkMesh(World& w);

	explicit WorldRenderer(SnapshotExchanger& exchanger) : m_exchanger(exchanger) {}


	void UploadPendingMeshData(WorldThread& wt);
	void DeleteMeshes(WorldThread& wt);
	void RenderWorld(const Camera& cam, World* w);
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

	void InitFogSys();

	void UpdateSnapshots();

	void UpdateFogSys(const Camera& cam);

private:

	void UploadPointLights(
		Shader& shader,
		const std::vector<PointLight>& lights,
		size_t count,
		const Camera& cam
	);


	void ExtractBrightPixels();
	RenderTexture2D& BlurBloom(int passCount);
	void RenderFinalPost(RenderTexture2D& bloomTexture);

	void DrawFullscreenTriangle();




	void UpdateLightVolume(const LightVolumeSnapshot& snapshot);


	std::unique_ptr<ChunkRenderabilitySnapshot> CreateChunkRenderabilitySnap() const;
private:

	SnapshotExchanger& m_exchanger;

	std::optional<DayNightSnapshot> m_dayNightSnapshot =
		DayNightSnapshot{
			.directionToSun = {0.0f, 1.0f, 0.0f},
			.dayFactor = 1.0f,
			.sunHeight = 1.0f,
			.sunIntensity = 1.0f,
			.skyStrength = 1.0f
		};

	std::unique_ptr<FogSystem> m_fogsys;

	std::unique_ptr<PointLightsSnapshot> m_pointLightsSnap;


	std::unordered_map<ChunkCoord, ChunkMesh, ChunkCoordHash> m_chunkMeshes;


	std::optional<Shader> baseShader;


	std::unique_ptr<ImageTexture2D> blockAtlas;
	

	unsigned int m_shadowFBO = 0;
	std::unique_ptr<DepthTexture2D> m_shadowDepthTexture;
	std::optional<Shader> m_shadowShader;

	unsigned int m_hdrFBO = 0;
	std::unique_ptr<RenderTexture2D> m_sceneTexture;
	unsigned int m_depthRBO = 0;


	unsigned int m_brightFBO = 0;
	std::unique_ptr<RenderTexture2D> m_brightTexture;
	unsigned int m_pingPongFBO[2] = { 0, 0 };
	std::array<std::unique_ptr<RenderTexture2D>, 2> m_pingpongTextures;


	std::unique_ptr<DataTexture3D> m_lightVolumeTexture;
	

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

	static constexpr int PENDING_MESH_BUDGET = 5;
};