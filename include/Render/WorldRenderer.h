#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Shader.h"
#include "Camera.h"

class World;

class WorldRenderer {
public:
	void RebuildDrityChunkMesh(World& w);
	void UploadPendingMeshData(World& w);
	void RenderWorld(const World& w, Shader& shader, const Camera& cam);
private:

};