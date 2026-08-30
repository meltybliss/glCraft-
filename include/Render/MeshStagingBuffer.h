#pragma once
#include <glad/glad.h>

#include <array>
#include <cstddef>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>


//staging bufferです
class MeshStagingBuffer {
public:

    static constexpr std::size_t SLOT_SIZE =
        8ull * 1024ull * 1024ull;//8 MB

    static constexpr std::size_t SLOT_COUNT = 16;

    static constexpr std::size_t TOTAL_SIZE =
        SLOT_SIZE * SLOT_COUNT;



    struct Reservation {

        std::size_t slotIndex = 0;
        std::size_t baseOffset = 0;//staging buffer全体から見た先頭位置
        std::byte* ptr = nullptr;//workerが書き込むcpu pointer


    };

    void Init();
    void Destroy();


    std::optional<Reservation> Acquire();

    void CancelPendingAcquires();

    void ReleaseWithoutGPU(std::size_t slotIndex);//捨てる場合


    void MarkGpuInFlight(std::size_t slotIndex, GLsync fence);

    void ReclaimCompleted();

    GLuint GetBuffer() const
    {
        return buffer;
    }


private:
    enum class SlotState
    {
        Free,
        //Workerが現在書いている
        CPUWriting,
        ///memcpy完了
        ReadyForGPU,

        //GPUが現在読んでいる
        GPUReading
    };

    struct Slot {

        SlotState state = SlotState::Free;

        GLsync fence = nullptr;

    };


private:

    GLuint buffer = 0;
    std::byte* mappedPtr = nullptr;

    std::array<Slot, SLOT_COUNT> slots;


    std::mutex mutex;
    std::condition_variable cv;
    bool acceptingReservations = true;

};
