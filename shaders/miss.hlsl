#ifndef MISS_HLSL
#define MISS_HLSL

#include "common.hlsli"

[shader("miss")]
void Miss(inout HitInfo payload)
{
    float t = 0.5f*(WorldRayDirection().y + 1.0f);
    payload.ShadedColor = (1.0f-t)*float4(1.0f, 1.0f, 1.0f, 1.0f) + t*float4(0.5f, 0.7f, 1.0f, 1.0f);
    payload.HitT = -1.0f;
}

#endif