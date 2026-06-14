#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "MeshData.h"

struct ChunkMesh {
	unsigned int vao;
	unsigned int vbo;
	unsigned int ebo;

	void Upload(const MeshData& data) {
		auto& vertices = data.vertices;
		auto& indices = data.indices;

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);

		//vbo
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(
			GL_ARRAY_BUFFER,
			vertices.size() * sizeof(float),
			vertices.data(),
			GL_STATIC_DRAW
		);

		//ebo
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			indices.size() * sizeof(uint8_t),
			indices.data(),
			GL_STATIC_DRAW
		);

		//vao
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			5 * sizeof(float),
			(void*)0
		);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(
			1,
			2,
			GL_FLOAT,
			GL_FALSE,
			5 * sizeof(float),
			(void*)(3 * sizeof(float))
		);
		glEnableVertexAttribArray(1);


		glBindVertexArray(0);//unbind

	}


	void Draw() {

	}

};