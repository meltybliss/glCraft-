#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "MeshData.h"

struct ChunkMesh {
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int ebo = 0;
	GLsizei indexCount = 0;

	void Upload(const MeshData& data) {
		auto& vertices = data.vertices;
		auto& indices = data.indices;

		indexCount = static_cast<GLsizei>(indices.size());

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
			indices.size() * sizeof(unsigned int),
			indices.data(),
			GL_STATIC_DRAW
		);

		constexpr GLsizei stride = 11 * sizeof(float);


		
		//vao
		//xyz
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			stride,
			(void*)0
		);
		glEnableVertexAttribArray(0);

		//UV
		glVertexAttribPointer(
			1,
			2,
			GL_FLOAT,
			GL_FALSE,
			stride,
			(void*)(3 * sizeof(float))
		);
		glEnableVertexAttribArray(1);

		//normal
		glVertexAttribPointer(
			2,
			3,
			GL_FLOAT,
			GL_FALSE,
			stride,
			(void*)(5 * sizeof(float))
		);
		glEnableVertexAttribArray(2);

		//block light level
		glVertexAttribPointer(
			3,
			1,
			GL_FLOAT,
			GL_FALSE,
			stride,
			(void*)(8 * sizeof(float))
		);
		glEnableVertexAttribArray(3);

		//sky light level
		glVertexAttribPointer(
			4,
			1,
			GL_FLOAT,
			GL_FALSE,
			stride,
			(void*)(9 * sizeof(float))
		);
		glEnableVertexAttribArray(4);


		//AO
		glVertexAttribPointer(
			5,
			1,
			GL_FLOAT,
			GL_FALSE,
			stride,
			(void*)(10 * sizeof(float))
		);
		glEnableVertexAttribArray(5);

		


		glBindVertexArray(0);//unbind

	}


	void Draw() const {
		if (vao == 0 || indexCount == 0) {
			return;
		}

		glBindVertexArray(vao);

		glDrawElements(
			GL_TRIANGLES,
			indexCount,
			GL_UNSIGNED_INT,
			nullptr
		);

		glBindVertexArray(0);
	}
	

	void DeleteGL() {
		if (ebo != 0) glDeleteBuffers(1, &ebo);
		if (vao != 0) glDeleteVertexArrays(1, &vao);
		if (vbo != 0) glDeleteBuffers(1, &vbo);


		vao = 0;
		vbo = 0;
		ebo = 0;
	}

};