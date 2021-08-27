
#ifndef RAYGEN_HLSL
#define RAYGEN_HLSL

#include "common.hlsli"

[shader("raygeneration")]
void RayGen()
{
    uint2 LaunchIndex = DispatchRaysIndex().xy;
    uint2 LaunchDimensions = DispatchRaysDimensions().xy;

    //Camera Position
    float3 origin = float3(0.0f, 0.0f, 0.0f);
    float focalLength = 1.0f;
    float2 uv = float2(LaunchIndex) / float2(LaunchDimensions);
    float2 ndc = uv * float2(2,-2) + float2(-1, +1);

    float3 direction = float3(ndc.xy, focalLength) - origin;


    // Setup the ray
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = direction;
    ray.TMin = 0.1f;
    ray.TMax = 1000.f;

    // Trace the ray
    HitInfo payload;
    payload.ShadedColor = float4(0.f, 0.f, 0.f, 0.f);
    payload.HitT = 0.f;

    TraceRay(
            gRaytracingAccelerationStructure,
            RAY_FLAG_NONE,
            0xffffffff,
            0,   /*Hit Gourp Index*/
            1,   /*Ray Stride*/
            0,   /*Miss Shader Index*/
            ray,
            payload);

    RWTexture2D<float4> output = gRTOutputs[gRenderTargetIdx];
    output[LaunchIndex.xy] = payload.ShadedColor;
}

#endif