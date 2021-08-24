#ifndef MISS_HLSL
#define MISS_HLSL

#include "common.hlsli"

[shader("miss")]
void Miss(inout HitInfo payload)
{
    payload.ShadedColorAndHitT = float4(0.2f, 0.2f, 0.2f, -1.f);
}

#endif