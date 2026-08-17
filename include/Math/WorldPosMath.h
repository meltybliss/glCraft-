#pragma once
#include "World/WorldPos.h"

WorldPos WorldPosLerp(
    const WorldPos& a,
    const WorldPos& b,
    double alpha)
{
    glm::i64vec3 blockDelta = b.block - a.block;
    glm::dvec3 localDelta = b.local - a.local;

    glm::dvec3 delta(
        static_cast<double>(blockDelta.x),
        static_cast<double>(blockDelta.y),
        static_cast<double>(blockDelta.z)
    );

    delta += localDelta;

    glm::dvec3 interpolatedDelta = delta * alpha;

    WorldPos result = a;

    result.local += interpolatedDelta;

    NormalizePosition(result);

    return result;
}