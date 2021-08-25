#ifndef MISS_HLSL
#define MISS_HLSL

#include "common.hlsli"

[shader("miss")]
void Miss(inout HitInfo payload)
{
    payload.ShadedColor = float4(0.2f, 0.2f, 0.2f, 0.0f);
    payload.HitT = -1.0f;
}

#endif