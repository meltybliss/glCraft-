#include "Render/WorldRenderer.h"
#include "World/WorldThread.h"
#include "Render/MeshBuilder.h"
#include "Core/ChunkJob.h"
#include "Core/WindowSize.h"
#include "Render/Texture.h"
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

	blockAtlas = std::make_unique<Texture>("assets/textures/block_atlas2.png");

}



void WorldRenderer::InitSkyShaderAndVAO() {

	glGenVertexArrays(1, &m_skyVAO);

	m_skyShader.emplace(
		"assets/Shaders/sky.vert",
		"assets/Shaders/sky.frag"
	);
}




void WorldRenderer::RenderSky(const Camera& cam) {

	const float aspect =
		static_cast<float>(WindowSize::windowWidth) /
		static_cast<float>(WindowSize::windowHeight);
	float tanHalfFov = std::tan(glm::radians(cam.fov));

	glDisable(GL_DEPTH_TEST);

	m_skyShader->Use();


	m_skyShader->SetFloat("aspect", aspect);
	m_skyShader->SetFloat("tanHalfFov", tanHalfFov);
	m_skyShader->SetVec3("cameraForward", cam.front);
	m_skyShader->SetVec3("cameraRight", cam.right);
	m_skyShader->SetVec3("cameraUp", cam.up);




	glBindVertexArray(m_skyVAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);


	glEnable(GL_DEPTH_TEST);
}


void WorldRenderer::InitShadownMap() {

	glGenFramebuffers(1, &m_shadowFBO);

	glGenTextures(1, &m_shadowDepthTexture);
	glBindTexture(GL_TEXTURE_2D, m_shadowDepthTexture);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT,
		SHADOW_WIDTH,
		SHADOW_HEIGHT,
		0,
		GL_DEPTH_COMPONENT,
		GL_FLOAT,
		nullptr

	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_COMPARE_MODE,
		GL_COMPARE_REF_TO_TEXTURE
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_COMPARE_FUNC,
		GL_LEQUAL
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_LINEAR
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_BORDER
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_BORDER
	);

	const float borderColor[] = {
		1.0f, 1.0f, 1.0f, 1.0f
	};

	glTexParameterfv(
		GL_TEXTURE_2D,
		GL_TEXTURE_BORDER_COLOR,
		borderColor
	);

	glBindFramebuffer(
		GL_FRAMEBUFFER,
		m_shadowFBO
	);

	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D,
		m_shadowDepthTexture,
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

	glGenTextures(1, &m_sceneTexture);
	glBindTexture(GL_TEXTURE_2D, m_sceneTexture);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA16F,
		WindowSize::windowWidth,
		WindowSize::windowHeight,
		0,
		GL_RGBA,
		GL_FLOAT,
		nullptr

	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR
	);


	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_LINEAR
	);

	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		m_sceneTexture,
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


	glGenTextures(1, &m_lightVolumeTexture);
	glBindTexture(GL_TEXTURE_3D, m_lightVolumeTexture);


	constexpr int channel_count = 4;

	using namespace LIGHT_VOLUME_SIZE;

	std::vector<float> emptyData(
		LIGHT_VOLUME_WIDTH *
		LIGHT_VOLUME_HEIGHT * 
		LIGHT_VOLUME_DEPTH *
		channel_count,
		0.0f
	);


	glTexImage3D(
		GL_TEXTURE_3D,
		0,
		GL_RGBA16F,
		LIGHT_VOLUME_WIDTH,
		LIGHT_VOLUME_HEIGHT,
		LIGHT_VOLUME_DEPTH,
		0,
		GL_RGBA,
		GL_FLOAT,
		emptyData.data()
	);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);



	glBindTexture(GL_TEXTURE_3D, 0);

}



void WorldRenderer::InitBloom() {
	

	//for Bright
	glGenFramebuffers(1, &m_brightFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_brightFBO);

	glGenTextures(1, &m_brightTexture);
	glBindTexture(GL_TEXTURE_2D, m_brightTexture);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA16F,
		(GLsizei)WindowSize::windowWidth,
		(GLsizei)WindowSize::windowHeight,
		0,
		GL_RGBA,
		GL_FLOAT,
		nullptr


	);


	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_LINEAR
	);

	
	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_EDGE
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_EDGE
	);


	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		m_brightTexture,
		0
	);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {

		std::cerr << "Bright framebuffer incomplete\n";

	}


	//for Blur

	glGenFramebuffers(2, m_pingPongFBO);
	glGenTextures(2, m_pingPongTextures);


	for (int i = 0; i < 2; ++i) {

		glBindFramebuffer(GL_FRAMEBUFFER, m_pingPongFBO[i]);

		glBindTexture(GL_TEXTURE_2D, m_pingPongTextures[i]);

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RGBA16F,
			(GLsizei)WindowSize::windowWidth,
			(GLsizei)WindowSize::windowHeight,
			0,
			GL_RGBA,
			GL_FLOAT,
			nullptr

		);

		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER,
			GL_LINEAR
		);

		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER,
			GL_LINEAR
		);


		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_WRAP_S,
			GL_CLAMP_TO_EDGE
		);

		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_WRAP_T,
			GL_CLAMP_TO_EDGE
		);


		glFramebufferTexture(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			m_pingPongTextures[i],
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

	glBindTexture(GL_TEXTURE_3D, m_lightVolumeTexture);

	glTexSubImage3D(
		GL_TEXTURE_3D,
		0,
		0, 0, 0,
		LIGHT_VOLUME_WIDTH,
		LIGHT_VOLUME_HEIGHT,
		LIGHT_VOLUME_DEPTH,
		GL_RGBA,
		GL_FLOAT,
		snapshot.pixels.data()

	);


	glBindTexture(GL_TEXTURE_3D, 0);

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


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_sceneTexture);


	DrawFullscreenTriangle();

}


unsigned int WorldRenderer::BlurBloom(int passCount) {

	if (passCount <= 0) {
		return m_brightTexture;
	}


	bool horizontal = true;
	int writeIndex = 0;


	unsigned int inputTexture = m_brightTexture;

	m_blurShader->Use();


	for (int pass = 0; pass < passCount; ++pass) {

		glBindFramebuffer(GL_FRAMEBUFFER, m_pingPongFBO[writeIndex]);

		glViewport(
			0,
			0,
			WindowSize::windowWidth,
			WindowSize::windowHeight
		);

		//trueÇ»ÇÁâ°Blur
		//falseÇ»ÇÁècBlur
		m_blurShader->SetInt(
			"uHorizontal",
			horizontal ? 1 : 0
		);


		glActiveTexture(GL_TEXTURE0);

		glBindTexture(
			GL_TEXTURE_2D,
			inputTexture
		);


		DrawFullscreenTriangle();

		inputTexture = m_pingPongTextures[writeIndex];

		writeIndex = 1 - writeIndex;

		horizontal =
			!horizontal;

	}
	return inputTexture;

}


void WorldRenderer::RenderFinalPost(unsigned int bloomTexture) {


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


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_sceneTexture);


	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bloomTexture);


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

	unsigned int bloomTexture = BlurBloom(20);

	RenderFinalPost(bloomTexture);

	glEnable(GL_DEPTH_TEST);
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

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	m_shadowShader->Use();
	


	glm::vec3 sunDirection =
		glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f));

	glm::vec3 shadowCenter = { 0.f, 0.f, 0.f };

	glm::vec3 lightPos =
		shadowCenter - sunDirection * 100.0f;

	glm::mat4 lightView = glm::lookAt(
		lightPos,
		shadowCenter,
		glm::vec3(0.0f, 1.0f, 0.0f)
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

		glm::mat4 model(1.0f);//identity matrix íPà çsóÒ

		int32_t cx = RestoreCxFromKey(key);
		int32_t cz = RestoreCzFromKey(key);

		WorldPos pos;
		pos.block = { static_cast<int64_t>(cx) * Chunk::CHUNK_WIDTH, 0, static_cast<int64_t>(cz) * Chunk::CHUNK_DEPTH };
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



void WorldRenderer::RenderWorld(const Camera& cam, World* w, const PointLightsSnapshot& snapshot) {

	using namespace LIGHT_VOLUME_SIZE;

	glViewport(0, 0, WindowSize::windowWidth, WindowSize::windowHeight);


	std::array<PointLight*, 16> pLights;
	size_t count = 0;


	baseShader->Use();


	glm::mat4 view = cam.GetViewMatrix();

	glm::mat4 projection = glm::perspective(
		glm::radians(cam.fov),
		WindowSize::windowWidth / WindowSize::windowHeight,
		0.1f,
		1000.f

	);


	glm::vec3 sunDirection =
		glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f));


	baseShader->SetFloat("u_skyStrength", 1.0f);

	baseShader->SetMat4("view", view);
	baseShader->SetMat4("projection", projection);

	baseShader->SetVec3("sunDirection", sunDirection);

	baseShader->SetMat4("lightSpaceMatrix", m_lightSpaceMatrix);

	WorldPos lightVolumePos;
	lightVolumePos.block = m_lightVolumeOrigin;
	lightVolumePos.local = glm::dvec3(0.0);

	glm::vec3 relativeOrigin = glm::vec3(
		GetRelativePos(cam.position, lightVolumePos)
	);

	baseShader->SetVec3("uLightVolumeOrigin", relativeOrigin);
	baseShader->SetVec3("uLightVolumeSize", glm::vec3(LIGHT_VOLUME_WIDTH, LIGHT_VOLUME_HEIGHT, LIGHT_VOLUME_DEPTH));

	blockAtlas->Bind(0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_shadowDepthTexture);//Ç±ÇÃtextureÇ‡Ç¢Ç∏ÇÍTextureå^Ç…Ç∑ÇÈ

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_3D, m_lightVolumeTexture);


	for (auto& [key, mesh] : m_chunkMeshes) {

		count = 0;
		pLights.fill(0);

		glm::mat4 model(1.0f);//identity matrix íPà çsóÒ

		int32_t cx = RestoreCxFromKey(key);
		int32_t cz = RestoreCzFromKey(key);

		WorldPos pos;
		pos.block = { static_cast<int64_t>(cx) * Chunk::CHUNK_WIDTH, 0, static_cast<int64_t>(cz) * Chunk::CHUNK_DEPTH };
		pos.local = glm::dvec3(0.0);

		glm::dvec3 relative = GetRelativePos(cam.position, pos);

		glm::vec3 drawPos = glm::vec3(relative);

		model = glm::translate(
			model,
			drawPos

		);

		baseShader->SetMat4("model", model);
		

		auto it = snapshot.pointLightsMap.find(key);
		if (it == snapshot.pointLightsMap.end()) continue;

		UploadPointLights(*baseShader, it->second.pointLights, it->second.count, cam);

		auto it2 = m_chunkMeshes.find(key);
		if (it2 == m_chunkMeshes.end()) continue;

		it2->second.Draw();

	}
}


void WorldRenderer::UploadPendingMeshData(WorldThread& wt) {
	PendingMesh out;

	
	while (wt.PopPendingMeshData(out)) {
		auto [it, inserted] = m_chunkMeshes.try_emplace(out.key);

		it->second.Upload(out.meshData);
		
	}

}



void WorldRenderer::DeleteMeshes(WorldThread& wt) {

	uint64_t key;

	while (wt.PopPendingDeleteMeshKey(key)) {
		auto it = m_chunkMeshes.find(key);
		if (it == m_chunkMeshes.end()) continue;

		it->second.DeleteGL();
		m_chunkMeshes.erase(key);

	}

}