#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "MeshData.h"

struct ChunkMesh {
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int ebo = 0;
	GLsizei indexCount = 0;

	std::size_t vertexCapacity = 0;
	std::size_t indexCapacity = 0;


	void EnsureCreated() {
		if (!vao) return;

		glGenVertexArrays(
			1,
			&vao
		);

		glGenBuffers(
			1,
			&vbo
		);

		glGenBuffers(
			1,
			&ebo
		);


		glBindVertexArray(vao);

		glBindBuffer(
			GL_ARRAY_BUFFER,
			vbo
		);


		constexpr GLsizei stride =
			11 * sizeof(float);


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




		glBindBuffer(
			GL_ELEMENT_ARRAY_BUFFER,
			ebo
		);

		glBindVertexArray(0);

	}


	static std::size_t GrowCapacity(
		std::size_t required
	)
	{
		std::size_t capacity =
			64 * 1024;


		while (capacity < required)
		{
			capacity *= 2;
		}


		return capacity;
	}



	void EnsureCapacity(
		std::size_t vertexBytes,
		std::size_t indexBytes
	) {

		if (vertexBytes > vertexCapacity) {

			vertexCapacity =
				GrowCapacity(
					vertexBytes
				);

			glBindBuffer(
				GL_COPY_WRITE_BUFFER,
				vbo
			);


			glBufferData(
				GL_COPY_WRITE_BUFFER,
				vertexCapacity,
				nullptr,
				GL_DYNAMIC_DRAW
			);

		}


		if (indexBytes > indexCapacity)
		{
			indexCapacity =
				GrowCapacity(
					indexBytes
				);


			glBindBuffer(
				GL_COPY_WRITE_BUFFER,
				ebo
			);


			glBufferData(
				GL_COPY_WRITE_BUFFER,
				indexCapacity,
				nullptr,
				GL_DYNAMIC_DRAW
			);
		}

	}


	void UploadFromStaging(
		GLuint stagingBuffer,

		std::size_t vertexOffset,
		std::size_t vertexBytes,

		std::size_t indexOffset,
		std::size_t indexBytes,

		GLsizei newIndexCount
	)
	{

		EnsureCreated();


		EnsureCapacity(
			vertexBytes,
			indexBytes
		);



		glBindBuffer(
			GL_COPY_READ_BUFFER,
			stagingBuffer

		);

		//staging->vbo
		glBindBuffer(
			GL_COPY_WRITE_BUFFER,
			vbo

		);

		if (vertexBytes > 0) {

			glCopyBufferSubData(
				GL_COPY_READ_BUFFER,
				GL_COPY_WRITE_BUFFER,

				vertexOffset,
				0,
				vertexBytes
			);


		}


		//staging->ebo
		glBindBuffer(
			GL_COPY_WRITE_BUFFER,
			ebo
		);

		if (indexBytes > 0)
		{
			glCopyBufferSubData(
				GL_COPY_READ_BUFFER,
				GL_COPY_WRITE_BUFFER,

				indexOffset,
				0,
				indexBytes
			);
		}


		indexCount = newIndexCount;


		glBindBuffer(
			GL_COPY_READ_BUFFER,
			0
		);

		glBindBuffer(
			GL_COPY_WRITE_BUFFER,
			0
		);


	}

	void Upload(const MeshData& data) {
		auto& vertices = data.vertices;
		auto& indices = data.indices;

		indexCount = static_cast<GLsizei>(indices.size());

		

		constexpr GLsizei stride = 11 * sizeof(float);

		
		if (vao == 0) {


			glGenVertexArrays(1, &vao);
			glGenBuffers(1, &vbo);
			glGenBuffers(1, &ebo);


			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
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


		
			

		}
		
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