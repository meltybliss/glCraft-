#include "Render/WorldRenderer.h"
#include "World/World.h"
#include "Render/MeshBuilder.h"

void WorldRenderer::RebuildChunkMesh(World& w) {
	for (auto& [key, c] : w.GetChunks()) {
		if (c->dirty) {
			MeshData data = MeshBuilder::BuildChunkMesh(w, *c);
			c->mesh.Upload(data);

			c->dirty = false;
		}

	}

}


void WorldRenderer::RenderWorld(const World& w, Shader& shader) {
	shader.Use();

	glm::mat4 view;

	glm::mat4 projection = glm::perspective(
		glm::radians(70.0f),
		800.0f / 600.0f,
		0.1f,
		1000.f

	);

	shader.SetMat4("view", view);
	shader.SetMat4("projection", projection);
	
	for (auto& [key, c] : w.GetChunks()) {


		c->mesh.Draw();
	}

}