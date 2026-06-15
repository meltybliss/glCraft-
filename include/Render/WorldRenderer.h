#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

class World;

class WorldRenderer {
public:
	void RebuildChunkMesh(World& w);

	void RenderWorld(const World& w);

};