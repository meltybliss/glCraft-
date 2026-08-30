#pragma once
#include <glad/glad.h>

#include <array>
#include <cstddef>
#include <condition_variable>
#include <cstdint>
#include <mutex>


//staging buffer‚Å‚·
class MeshStagingBuffer {
public:

    static constexpr std::size_t SLOT_SIZE =
        8ull * 1024ull * 1024ull;//8 MB

    static constexpr std::size_t SLOT_COUNT = 16;

    static constexpr std::size_t TOTAL_SIZE =
        SLOT_SIZE * SLOT_COUNT;



    struct Reservation {

        std::size_t slotIndex = 0;
        std::size_t baseOffset = 0;//staging buffer‘S‘Ì‚©‚çŒ©‚½æ“ªˆÊ’u
        std::byte* ptr = nullptr;//worker‚ª‘‚«‚Şcpu pointer


    };

    void Init();
    void Destroy();


    Reservation Acquire();

    void ReleaseWithoutGPU(std::size_t slotIndex);//Ì‚Ä‚éê‡


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
        //Worker‚ªŒ»İ‘‚¢‚Ä‚¢‚é
        CPUWriting,

        //GPU‚ªŒ»İ“Ç‚ñ‚Å‚¢‚é
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

};