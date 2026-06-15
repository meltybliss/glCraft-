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


void WorldRenderer::RenderWorld(const World& w) {
	for (auto& [key, c] : w.GetChunks()) {
		c->mesh.Draw();
	}

}