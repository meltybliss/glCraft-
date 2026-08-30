#include "Render/MeshStagingBuffer.h"

#include <cstring>
#include <stdexcept>


void MeshStagingBuffer::Init() {
	{
		std::lock_guard<std::mutex> lock(mutex);
		acceptingReservations = true;
	}

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


std::optional<MeshStagingBuffer::Reservation>
MeshStagingBuffer::Acquire()
{
	std::unique_lock<std::mutex>
		lock(mutex);


	std::size_t freeIndex =
		SLOT_COUNT;


	cv.wait(
		lock,

		[this, &freeIndex]()
		{
			if (!acceptingReservations) {
				return true;
			}

			for (
				std::size_t i = 0;
				i < SLOT_COUNT;
				++i
				)
			{
				if (
					slots[i].state ==
					SlotState::Free
					)
				{
					freeIndex = i;

					return true;
				}
			}


			return false;
		}
	);

	if (!acceptingReservations) {
		return std::nullopt;
	}


	slots[freeIndex].state =
		SlotState::CPUWriting;


	Reservation result;

	result.slotIndex =
		freeIndex;

	result.baseOffset =
		freeIndex
		* SLOT_SIZE;

	result.ptr =
		mappedPtr
		+ result.baseOffset;


	return result;
}


void MeshStagingBuffer::CancelPendingAcquires()
{
	{
		std::lock_guard<std::mutex> lock(mutex);
		acceptingReservations = false;
	}

	cv.notify_all();
}



void MeshStagingBuffer::ReleaseWithoutGPU(
	std::size_t slotIndex
)
{
	{
		std::lock_guard<std::mutex>
			lock(mutex);


		slots[slotIndex].state =
			SlotState::Free;

		slots[slotIndex].fence =
			nullptr;
	}


	cv.notify_one();
}


void MeshStagingBuffer::MarkGpuInFlight(
	std::size_t slotIndex,
	GLsync fence
)
{
	std::lock_guard<std::mutex>
		lock(mutex);


	slots[slotIndex].state =
		SlotState::GPUReading;

	slots[slotIndex].fence =
		fence;
}



void MeshStagingBuffer::ReclaimCompleted()
{
	bool releasedAny = false;


	{
		std::lock_guard<std::mutex>
			lock(mutex);


		for (auto& slot : slots)
		{
			if (
				slot.state !=
				SlotState::GPUReading
				)
			{
				continue;
			}


			const GLenum result =
				glClientWaitSync(
					slot.fence,
					0,
					0
				);


			if (
				result ==
				GL_ALREADY_SIGNALED
				||
				result ==
				GL_CONDITION_SATISFIED
				)
			{
				glDeleteSync(
					slot.fence
				);


				slot.fence =
					nullptr;

				slot.state =
					SlotState::Free;


				releasedAny =
					true;
			}
		}
	}


	if (releasedAny)
	{
		cv.notify_all();
	}
}


void MeshStagingBuffer::Destroy()
{
	if (buffer == 0)
	{
		return;
	}


	//終了時だけなので待ってよい
	glFinish();


	for (auto& slot : slots)
	{
		if (slot.fence != nullptr)
		{
			glDeleteSync(
				slot.fence
			);

			slot.fence =
				nullptr;
		}


		slot.state =
			SlotState::Free;
	}


	glBindBuffer(
		GL_COPY_READ_BUFFER,
		buffer
	);


	glUnmapBuffer(
		GL_COPY_READ_BUFFER
	);


	glBindBuffer(
		GL_COPY_READ_BUFFER,
		0
	);


	glDeleteBuffers(
		1,
		&buffer
	);


	buffer = 0;
	mappedPtr = nullptr;
}
