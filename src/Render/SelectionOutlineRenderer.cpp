#include "Render/SelectionOutlineRenderer.h"
#include "Render/Camera.h"

void SelectionOutlineRenderer::Init() {
	constexpr float e = 0.002f;
	constexpr float min = -e;
	constexpr float max = 1.0f + e;

	std::vector<float> vertices{
		//front
		min, min, min, max, min, min,
		max, min, min, max, max, min,
		max, max, min, min, max, min,
		min, max, min, min, min, min,

		//back
		min, min, max, max, min, max,
		max, min, max, max, max, max,
		max, max, max, min, max, max,
		min, max, max, min, min, max,

		// “ñ‚Â‚Ì–Ê‚ð‚Â‚È‚®•Ó
		min, min, min,   min, min, max,
		max, min, min,   max, min, max,
		max, max, min,   max, max, max,
		min, max, min,   min, max, max,


	};

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	glBufferData(
		GL_ARRAY_BUFFER,
		vertices.size() * sizeof(float),
		vertices.data(),
		GL_STATIC_DRAW

	);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 3,
		(void*)0
	);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

}


void SelectionOutlineRenderer::RenderOutline(
	int64_t hitX, 
	int64_t hitY, 
	int64_t hitZ,
	Camera& cam, 
	Shader& shader) const {

	shader.Use();

	glm::mat4 view = cam.GetViewMatrix();


	glm::mat4 proj = glm::perspective(
		glm::radians(70.0f),
		800.0f / 600.0f,
		0.1f,
		1000.f

	);

	glm::mat4 model(1.0f);
	model = glm::translate(
		model,
		glm::vec3(
			static_cast<float>(hitX), 
			static_cast<float>(hitY), 
			static_cast<float>(hitZ))
	);


	shader.SetMat4("view", view);
	shader.SetMat4("projection", proj);
	shader.SetMat4("model", model);
	shader.SetVec4("u_Color", { 1.0f, 1.0f, 1.0f, 1.0f });

	glBindVertexArray(vao);

	glDrawArrays(GL_LINES, 0, 24);
	glBindVertexArray(0);
}