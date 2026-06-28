#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Render/ChunkMesh.h"
#include "Shader.h"
#include "Camera.h"
#include <unordered_map>
#include <stdint.h>

class WorldThread;

class WorldRenderer {
public:
	//void RebuildDrityChunkMesh(World& w);
	void UploadPendingMeshData(WorldThread& wt);
	void RenderWorld(Shader& shader, const Camera& cam);
private:

	std::unordered_map<uint64_t, ChunkMesh> m_chunkMeshes;

};