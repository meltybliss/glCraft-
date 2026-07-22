#include "Render/WorldRenderer.h"
#include "World/WorldThread.h"
#include "Render/MeshBuilder.h"
#include "Core/ChunkJob.h"
#include <iostream>

/*void WorldRenderer::RebuildDrityChunkMesh(World& w) {
	for (auto& [key, c] : w.GetChunks()) {
		if (c->dirty) {


			w.EnqueueLightJobFrom_Outside(*c);

			c->dirty = false;
		}

	}

}*/



void WorldRenderer::InitSkyRender() {

	glGenVertexArrays(1, &m_skyVAO);

	m_skyShader.emplace(
		"assets/Shaders/sky.vert",
		"assets/Shaders/sky.frag"
	);
}


void WorldRenderer::RenderSky() {


	glDisable(GL_DEPTH_TEST);

	m_skyShader->Use();

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
		GL_TEXTURE_MIN_FILTER,
		GL_NEAREST
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_NEAREST
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

		glm::mat4 model(1.0f);//identity matrix 単位行列

		int32_t cx = RestoreCxFromKey(key);
		int32_t cz = RestoreCzFromKey(key);

		model = glm::translate(
			model,
			glm::vec3(
				cx * Chunk::CHUNK_WIDTH,
				0,
				cz * Chunk::CHUNK_DEPTH
			)

		);

		m_shadowShader->SetMat4("model", model);


		mesh.Draw();

	}


	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}



void WorldRenderer::RenderWorld(Shader& shader, const Camera& cam) {

	glViewport(0, 0, 800, 600);

	shader.Use();

	glm::mat4 view = cam.GetViewMatrix();

	glm::mat4 projection = glm::perspective(
		glm::radians(70.0f),
		800.0f / 600.0f,
		0.1f,
		1000.f

	);


	glm::vec3 sunDirection =
		glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f));

	

	shader.SetFloat("u_skyStrength", 1.0f);

	shader.SetMat4("view", view);
	shader.SetMat4("projection", projection);

	shader.SetVec3("sunDirection", sunDirection);

	shader.SetMat4("lightSpaceMatrix", m_lightSpaceMatrix);


	//TODO ふつうこれはinitで一度だけにしたい。そのためにbaseShaderも普通にrendererが所有にする.
	shader.SetInt("shadowMap", 1);
	//

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_shadowDepthTexture);


	for (auto& [key, mesh] : m_chunkMeshes) {

		glm::mat4 model(1.0f);//identity matrix 単位行列

		int32_t cx = RestoreCxFromKey(key);
		int32_t cz = RestoreCzFromKey(key);

		model = glm::translate(model,
			glm::vec3(
				cx * Chunk::CHUNK_WIDTH,
				0,
				cz * Chunk::CHUNK_DEPTH
			)

		);

		shader.SetMat4("model", model);

		auto it = m_chunkMeshes.find(key);
		if (it == m_chunkMeshes.end()) continue;

		it->second.Draw();

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