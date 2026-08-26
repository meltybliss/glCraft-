#include "Render/WorldRenderer.h"
#include "World/WorldThread.h"
#include "Render/MeshBuilder.h"
#include "Core/ChunkJob.h"
#include "Core/WindowSize.h"
#include <iostream>

/*void WorldRenderer::RebuildDrityChunkMesh(World& w) {
	for (auto& [key, c] : w.GetChunks()) {
		if (c->dirty) {


			w.EnqueueLightJobFrom_Outside(*c);

			c->dirty = false;
		}

	}

}*/


void WorldRenderer::InitBaseShader() {

	baseShader.emplace(
		"assets/Shaders/basic.vert",
		"assets/Shaders/basic.frag"
	);


	UVMinMax minMax = MeshBuilder::GetTorchUVMinMax();


	baseShader->Use();
	baseShader->SetInt("u_Texture", 0);
	baseShader->SetInt("shadowMap", 1);
	baseShader->SetInt("uLightVolumeTexture", 2);

	baseShader->SetVec2("torchMinUV", minMax.min);
	baseShader->SetVec2("torchMaxUV", minMax.max);


	baseShader->SetVec3("dayHorizonColor", dayHorizonColor);
	baseShader->SetVec3("dayTopColor", datTopColor);
	baseShader->SetVec3("nightHorizonColor", nightHorizonColor);
	baseShader->SetVec3("nightTopColor", nightTopColor);

	blockAtlas = std::make_unique<ImageTexture2D>("assets/textures/block_atlas2.png");

}



void WorldRenderer::InitSkyShaderAndVAO() {

	glGenVertexArrays(1, &m_skyVAO);

	m_skyShader.emplace(
		"assets/Shaders/sky.vert",
		"assets/Shaders/sky.frag"
	);

	m_skyShader->Use();
	m_skyShader->SetVec3("dayHorizonColor", dayHorizonColor);
	m_skyShader->SetVec3("dayTopColor", datTopColor);
	m_skyShader->SetVec3("nightHorizonColor", nightHorizonColor);
	m_skyShader->SetVec3("nightTopColor", nightTopColor);
}



void WorldRenderer::InitFogSys() {


	m_fogsys = std::make_unique<FogSystem>();

	baseShader->Use();

	baseShader->SetInt("uRenderableDistancesTexture", 3);
	baseShader->SetFloat("uFogEndMargin", 8.0f);
	baseShader->SetFloat("uFogWidth", 24.0f);


}



void WorldRenderer::UpdateFogSys(const Camera& cam) {

	auto snap = CreateChunkRenderabilitySnap();

	m_fogsys->UpdateFog(cam.position, *snap);

}



void WorldRenderer::RenderSky(const Camera& cam) {

	const float aspect =
		static_cast<float>(WindowSize::windowWidth) /
		static_cast<float>(WindowSize::windowHeight);
	float tanHalfFov = std::tan(glm::radians(cam.fov) * 0.5f);

	glDisable(GL_DEPTH_TEST);

	m_skyShader->Use();


	m_skyShader->SetFloat("aspect", aspect);
	m_skyShader->SetFloat("tanHalfFov", tanHalfFov);
	m_skyShader->SetVec3("cameraForward", cam.front);
	m_skyShader->SetVec3("cameraRight", cam.right);
	m_skyShader->SetVec3("cameraUp", cam.up);

	m_skyShader->SetFloat("uDayFactor", m_dayNightSnapshot->dayFactor);
	m_skyShader->SetVec3("sunDirection", m_dayNightSnapshot->directionToSun);

	glBindVertexArray(m_skyVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);


	glEnable(GL_DEPTH_TEST);
}


void WorldRenderer::InitShadownMap() {

	glGenFramebuffers(1, &m_shadowFBO);

	
	m_shadowDepthTexture = std::make_unique<DepthTexture2D>(SHADOW_WIDTH, SHADOW_HEIGHT);

	glBindFramebuffer(
		GL_FRAMEBUFFER,
		m_shadowFBO
	);

	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D,
		m_shadowDepthTexture->GetID(),
		0
	);


	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER)
		!= GL_FRAMEBUFFER_COMPLETE) {

		std::cerr << "Shadow FBO incomplete\n";
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	m_shadowShader.emplace(
		"assets/Shaders/shadow_depth.vert",
		"assets/Shaders/shadow_depth.frag"

	);

	
}



void WorldRenderer::InitHDRFrameBuffer() {

	glGenFramebuffers(1, &m_hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);


	m_sceneTexture = std::make_unique<RenderTexture2D>(
		WindowSize::windowWidth,
		WindowSize::windowHeight,
		GL_RGBA16F,
		GL_RGBA,
		GL_FLOAT
	);

	

	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		m_sceneTexture->GetID(),
		0
	);

	glGenRenderbuffers(1, &m_depthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);

	glRenderbufferStorage(
		GL_RENDERBUFFER,
		GL_DEPTH_COMPONENT24,
		WindowSize::windowWidth,
		WindowSize::windowHeight

	);


	glFramebufferRenderbuffer(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER,
		m_depthRBO
	);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "HDR framebuffer incomplete\n";
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}




void WorldRenderer::InitLightVolumeTexture() {


	m_lightVolumeTexture = std::make_unique<DataTexture3D>(GL_RGBA16F, GL_RGBA, GL_FLOAT);

}



void WorldRenderer::InitBloom() {
	

	//for Bright
	glGenFramebuffers(1, &m_brightFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_brightFBO);

	m_brightTexture = std::make_unique<RenderTexture2D>(WindowSize::windowWidth, WindowSize::windowHeight, GL_RGBA16F, GL_RGBA, GL_FLOAT);

	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		m_brightTexture->GetID(),
		0
	);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {

		std::cerr << "Bright framebuffer incomplete\n";

	}


	//for Blur

	glGenFramebuffers(2, m_pingPongFBO);
	

	m_pingpongTextures[0] =
		std::make_unique<RenderTexture2D>(WindowSize::windowWidth, WindowSize::windowHeight, GL_RGBA16F, GL_RGBA, GL_FLOAT);

	m_pingpongTextures[1] =
		std::make_unique<RenderTexture2D>(WindowSize::windowWidth, WindowSize::windowHeight, GL_RGBA16F, GL_RGBA, GL_FLOAT);

	for (int i = 0; i < 2; ++i) {

		glBindFramebuffer(GL_FRAMEBUFFER, m_pingPongFBO[i]);


		glFramebufferTexture(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			m_pingpongTextures[i]->GetID(),
			0
		);



		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {

			std::cerr << "PingPong framebuffer" << i << " incomplete\n";

		}


	}



	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//Shader
	m_brightShader.emplace(
		"assets/Shaders/post.vert",
		"assets/Shaders/bright.frag"

	);

	m_blurShader.emplace(

		"assets/Shaders/post.vert",
		"assets/Shaders/blur.frag"
	);

	m_postShader.emplace(
		"assets/Shaders/post.vert",
		"assets/Shaders/post.frag"
	);

	glGenVertexArrays(
		1,
		&m_postVAO
	);



	m_brightShader->Use();
	m_brightShader->SetInt(
		"uSceneTexture",
		0

	);


	m_blurShader->Use();
	m_blurShader->SetInt(
		"uImage",
		0
	);


	m_postShader->Use();
	m_postShader->SetInt(
		"uSceneTexture",
		0
	);

	m_postShader->SetInt(
		"uBloomTexture",
		1
	);

}



void WorldRenderer::UpdateLightVolume(const LightVolumeSnapshot& snapshot) {


	constexpr int channelCount = 4;

	using namespace LIGHT_VOLUME_SIZE;


	constexpr std::size_t expectedSize =
		static_cast<std::size_t>(LIGHT_VOLUME_WIDTH) *
		static_cast<std::size_t>(LIGHT_VOLUME_HEIGHT) *
		static_cast<std::size_t>(LIGHT_VOLUME_DEPTH) *
		channelCount;

	if (snapshot.pixels.size() != expectedSize) {
		std::cerr
			<< "Invalid LightVolumeSnapshot size: "
			<< snapshot.pixels.size()
			<< " expected: "
			<< expectedSize
			<< '\n';

		return;
	}

	m_lightVolumeOrigin = snapshot.origin;

	m_lightVolumeTexture->Bind();

	m_lightVolumeTexture->UpdateSub(snapshot.pixels.data());

	m_lightVolumeTexture->Unbind();

}


void WorldRenderer::ExtractBrightPixels() {


	glBindFramebuffer(GL_FRAMEBUFFER, m_brightFBO);

	glViewport(
		0,
		0,
		WindowSize::windowWidth,
		WindowSize::windowHeight

	);


	m_brightShader->Use();

	m_brightShader->SetFloat("uThreshold", 1.f);


	m_sceneTexture->Bind();


	DrawFullscreenTriangle();

}



RenderTexture2D& WorldRenderer::BlurBloom(int passCount) {

	if (passCount <= 0) {
		return *m_brightTexture;
	}


	bool horizontal = true;
	int writeIndex = 0;


	RenderTexture2D* inputTexture = m_brightTexture.get();

	m_blurShader->Use();


	for (int pass = 0; pass < passCount; ++pass) {

		glBindFramebuffer(GL_FRAMEBUFFER, m_pingPongFBO[writeIndex]);

		glViewport(
			0,
			0,
			WindowSize::windowWidth,
			WindowSize::windowHeight
		);

		//trueなら横Blur
		//falseなら縦Blur
		m_blurShader->SetInt(
			"uHorizontal",
			horizontal ? 1 : 0
		);



		inputTexture->Bind();


		DrawFullscreenTriangle();

		inputTexture = m_pingpongTextures[writeIndex].get();

		writeIndex = 1 - writeIndex;

		horizontal =
			!horizontal;

	}
	return *inputTexture;

}


void WorldRenderer::RenderFinalPost(RenderTexture2D& bloomTexture) {


	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(
		0,
		0,
		WindowSize::windowWidth,
		WindowSize::windowHeight
	);


	m_postShader->Use();

	m_postShader->SetFloat(
		"uExposure",
		1.f
	);

	m_postShader->SetFloat(
		"uBloomStrength",
		1.f
	);


	m_sceneTexture->Bind();

	bloomTexture.Bind(1);


	DrawFullscreenTriangle();

}



void WorldRenderer::DrawFullscreenTriangle() {


	glBindVertexArray(m_postVAO);

	glDrawArrays(GL_TRIANGLES, 0, 3);


	glBindVertexArray(0);
}



void WorldRenderer::BeginHDRScene() {

	glBindFramebuffer(GL_FRAMEBUFFER, m_hdrFBO);
	

	glViewport(
		0,
		0,
		WindowSize::windowWidth,
		WindowSize::windowHeight
	);


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}



void WorldRenderer::EndHDRScene() {

	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	glDisable(GL_DEPTH_TEST);

	ExtractBrightPixels();

	RenderTexture2D& bloomTexture = BlurBloom(20);

	RenderFinalPost(bloomTexture);

	glEnable(GL_DEPTH_TEST);
}


void WorldRenderer::UpdateSnapshots() {


	if (auto opt = m_exchanger.AcquireDayNightSnap()) {

		m_dayNightSnapshot =
			std::move(opt);
	}



	if (auto opt = m_exchanger.AcquirePointLightSnap())
	{
		m_pointLightsSnap =
			std::move(opt);
	}



	if (auto opt = m_exchanger.AcquireLightVolumeSnap())
	{
		UpdateLightVolume(
			*opt
		);

	}

}


void WorldRenderer::UploadPointLights(
	Shader& shader,
	const std::vector<PointLight>& lights,
	size_t count,
	const Camera& cam
) {

	shader.SetInt("uPointLightCount", static_cast<int>(count));

	for (size_t i = 0; i < count; ++i) {

		const auto& light = lights[i];
		

		std::string uniformName =
			"uPointLights[" +
			std::to_string(i) +
			"]";

		WorldPos lightPos;
		lightPos.block = light.position;
		lightPos.local = glm::dvec3(0.5);

		glm::dvec3 relative = GetRelativePos(cam.position, lightPos);

		shader.SetVec3(
			(uniformName + ".position").c_str(),
			glm::vec3(relative)
		);

		shader.SetVec3(
			(uniformName + ".color").c_str(),
			light.color
		);

		shader.SetFloat(
			(uniformName + ".radius").c_str(),
			light.radius
		);

		shader.SetFloat(
			(uniformName + ".intensity").c_str(),
			light.intensity
		);

	}

}


void WorldRenderer::RenderShadowPass(const Camera& cam) {

	if (!m_dayNightSnapshot) return;

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	m_shadowShader->Use();
	



	glm::vec3 sunDirection = m_dayNightSnapshot->directionToSun;
	glm::vec3 sunDir =
		glm::normalize(sunDirection);

	glm::vec3 up(0.0f, 1.0f, 0.0f);


	glm::vec3 shadowCenter = { 0.f, 0.f, 0.f };

	glm::vec3 lightPos =
		shadowCenter + sunDir * 100.0f;

	glm::mat4 lightView = glm::lookAt(
		lightPos,
		shadowCenter,
		up
	);

	constexpr float shadowRange = 80.0f;

	glm::mat4 lightProjection = glm::ortho(
		-shadowRange,
		shadowRange,
		-shadowRange,
		shadowRange,
		1.0f,
		250.0f//far
	);

	m_lightSpaceMatrix =
		lightProjection * lightView;


	m_shadowShader->SetMat4(
		"lightSpaceMatrix",
		m_lightSpaceMatrix
	);



	for (auto& [key, mesh] : m_chunkMeshes) {

		glm::mat4 model(1.0f);//identity matrix 単位行列

		WorldPos pos;
		pos.block = { key.x * Chunk::CHUNK_WIDTH, 0, key.z * Chunk::CHUNK_DEPTH };
		pos.local = glm::dvec3(0.0);

		glm::dvec3 relative = GetRelativePos(cam.position, pos);

		glm::vec3 drawPos = glm::vec3(relative);

		model = glm::translate(
			model,
			drawPos

		);

		m_shadowShader->SetMat4("model", model);


		mesh.Draw();

	}


	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}



void WorldRenderer::RenderWorld(const Camera& cam, World* w) {

	using namespace LIGHT_VOLUME_SIZE;

	glViewport(0, 0, WindowSize::windowWidth, WindowSize::windowHeight);

	


	std::array<PointLight*, 16> pLights{};
	size_t count = 0;


	baseShader->Use();


	glm::mat4 view = cam.GetViewMatrix();

	glm::mat4 projection = glm::perspective(
		glm::radians(cam.fov),
		static_cast<float>(WindowSize::windowWidth) /
		static_cast<float>(WindowSize::windowHeight),
		0.1f,
		1000.f

	);

	baseShader->SetFloat("u_skyStrength", m_dayNightSnapshot->skyStrength);

	baseShader->SetMat4("view", view);
	baseShader->SetMat4("projection", projection);

	baseShader->SetVec3("sunDirection", m_dayNightSnapshot->directionToSun);

	baseShader->SetFloat("uSunIntensity", m_dayNightSnapshot->sunIntensity);

	baseShader->SetMat4("lightSpaceMatrix", m_lightSpaceMatrix);


	baseShader->SetFloat("uDayFactor", m_dayNightSnapshot->dayFactor);

	WorldPos lightVolumePos;
	lightVolumePos.block = m_lightVolumeOrigin;
	lightVolumePos.local = glm::dvec3(0.0);

	glm::vec3 relativeOrigin = glm::vec3(
		GetRelativePos(cam.position, lightVolumePos)
	);

	baseShader->SetVec3("uLightVolumeOrigin", relativeOrigin);
	baseShader->SetVec3("uLightVolumeSize", glm::vec3(LIGHT_VOLUME_WIDTH, LIGHT_VOLUME_HEIGHT, LIGHT_VOLUME_DEPTH));

	blockAtlas->Bind(0);
	m_shadowDepthTexture->Bind(1);
	m_lightVolumeTexture->Bind(2);
	m_fogsys->Bind(3);


	for (auto& [key, mesh] : m_chunkMeshes) {

		count = 0;
		pLights.fill(0);

		glm::mat4 model(1.0f);//identity matrix 単位行列

		WorldPos pos;
		pos.block = { key.x * Chunk::CHUNK_WIDTH, 0, key.z * Chunk::CHUNK_DEPTH };
		pos.local = glm::dvec3(0.0);

		glm::dvec3 relative = GetRelativePos(cam.position, pos);

		glm::vec3 drawPos = glm::vec3(relative);

		model = glm::translate(
			model,
			drawPos

		);

		
		baseShader->SetMat4("model", model);
		
		if (m_pointLightsSnap) {
			auto it = m_pointLightsSnap->pointLightsMap.find(key);
			if (it != m_pointLightsSnap->pointLightsMap.end()) {
				UploadPointLights(
					*baseShader,
					it->second.pointLights,
					it->second.count,
					cam
				);
			}
			else {
				baseShader->SetInt("uPointLightCount", 0);
			}
		}
		else {
			baseShader->SetInt("uPointLightCount", 0);
		}

		mesh.Draw();

	}
}


void WorldRenderer::UploadPendingMeshData(WorldThread& wt) {
	PendingMesh out;

	
	while (wt.PopPendingMeshData(out)) {
		auto [it, inserted] = m_chunkMeshes.try_emplace(out.key);

		if (!inserted) {
			it->second.DeleteGL();
		}
		it->second.Upload(out.meshData);
		
	}

}



void WorldRenderer::DeleteMeshes(WorldThread& wt) {

	ChunkCoord key;

	while (wt.PopPendingDeleteMeshKey(key)) {
		auto it = m_chunkMeshes.find(key);
		if (it == m_chunkMeshes.end()) continue;

		it->second.DeleteGL();
		m_chunkMeshes.erase(key);

	}

}



std::unique_ptr<ChunkRenderabilitySnapshot>
WorldRenderer::CreateChunkRenderabilitySnap() const {

	std::unique_ptr<ChunkRenderabilitySnapshot> result = 
		std::make_unique<ChunkRenderabilitySnapshot>();

	result->renderableChunks.reserve(m_chunkMeshes.size());


	for (const auto& [key, mesh] : m_chunkMeshes) {

		result->renderableChunks.insert(key);

	}


	return std::move(result);
}
