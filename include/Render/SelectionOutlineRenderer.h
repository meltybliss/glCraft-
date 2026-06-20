#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "World/RaycastHit.h"
#include "Render/Shader.h"
#include <vector>

class Camera;


class SelectionOutlineRenderer {
public:

	void Init();
	void RenderOutline(int64_t hitX, int64_t hitY, int64_t hitZ, Camera& cam, Shader& shader) const;

private:

	unsigned int vao = 0;
	unsigned int vbo = 0;


};