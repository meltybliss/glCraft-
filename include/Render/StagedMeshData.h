#pragma once
#include <stdint.h>
#include <algorithm>

struct StagedMeshData
{
    //staging bufferの何番slotか
    std::size_t slotIndex = 0;


    //staging buffer全体の中で
    //vertexがどこから始まるか
    std::size_t vertexOffset = 0;

    //vertexが何byteあるか
    std::size_t vertexBytes = 0;


    //indexがどこから始まるか
    std::size_t indexOffset = 0;

    //indexが何byteあるか
    std::size_t indexBytes = 0;


    //glDrawElementsで必要
    uint32_t indexCount = 0;
};