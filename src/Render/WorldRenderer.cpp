#include "Render/WorldRenderer.h"
#include "World/WorldThread.h"
#include "Render/MeshBuilder.h"
#include "Core/ChunkJob.h"

/*void WorldRenderer::RebuildDrityChunkMesh(World& w) {
	for (auto& [key, c] : w.GetChunks()) {
		if (c->dirty) {


			w.EnqueueLightJobFrom_Outside(*c);

			c->dirty = false;
		}

	}

}*/


void WorldRenderer::RenderWorld(Shader& shader, const Camera& cam) {
	shader.Use();

	glm::mat4 view = cam.GetViewMatrix();

	glm::mat4 projection = glm::perspective(
		glm::radians(70.0f),
		800.0f / 600.0f,
		0.1f,
		1000.f

	);


	shader.SetFloat("u_skyStrength", 1.0f);

	shader.SetMat4("view", view);
	shader.SetMat4("projection", projection);
	
	for (auto& [key, mesh] : m_chunkMeshes) {

		glm::mat4 model(1.0f);//identity matrix ’PˆÊs—ñ

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
