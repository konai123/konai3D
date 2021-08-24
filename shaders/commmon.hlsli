#ifndef COMMON_HLSLI
#define COMMON_HLSLI

#include "descriptor_table.hlsli"
#include "shader_type.hlsli"
#include "sampler.hlsli"

cbuffer PerFrame : register(b1, space0)
{
    float4x4 gViewMatrix;
    float4 gViewOriginAndTanHalfFovY;
    float2 gResolution;
};

StructuredBuffer<Material> gMaterials : register(t1, space0);
Texture2D gRaytracingAccelerationStructure : register(t2, space0);
Texture2D gRTOutput : register(u1, space0);

#endif