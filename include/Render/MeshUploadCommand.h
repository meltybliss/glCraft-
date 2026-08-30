#pragma once

#include "glad/glad.h"
#include <stdint.h>
#include <algorithm>

class Chunk;

struct MeshUploadCommand
{
    Chunk* chunk = nullptr;

    uint64_t generation = 0;

    std::size_t stagingSlot = 0;


    std::size_t vertexOffset = 0;
    std::size_t vertexBytes = 0;


    std::size_t indexOffset = 0;
    std::size_t indexBytes = 0;


    GLsizei indexCount = 0;
};