#include "Render/MeshStagingBuffer.h"

#include <cstring>
#include <stdexcept>


void MeshStagingBuffer::Init() {

	glGenBuffers(1, &buffer);

	glBindBuffer(
		GL_COPY_READ_BUFFER,
		buffer
	);


	constexpr GLbitfield storageFlags =
		GL_MAP_WRITE_BIT |
		GL_MAP_PERSISTENT_BIT |
		GL_MAP_COHERENT_BIT;



	glBufferStorage(

		GL_COPY_READ_BUFFER,
		TOTAL_SIZE,
		nullptr,
		storageFlags
	);

	constexpr GLbitfield mapFlags =
		GL_MAP_WRITE_BIT |
		GL_MAP_PERSISTENT_BIT |
		GL_MAP_COHERENT_BIT;


	mappedPtr =
		static_cast<std::byte*>(
			glMapBufferRange(
				GL_COPY_READ_BUFFER,
				0,
				TOTAL_SIZE,
				mapFlags


			)

		);



	if (mappedPtr == nullptr)
	{
		throw std::runtime_error(
			"failed to map mesh staging buffer"
		);
	}



	glBindBuffer(
		GL_COPY_READ_BUFFER,
		0
	);

}