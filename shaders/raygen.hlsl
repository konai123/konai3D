
#ifndef RAYGEN_HLSL
#define RAYGEN_HLSL

#include "common.hlsli"

[shader("raygeneration")]
void RayGen()
{
    uint2 LaunchIndex = DispatchRaysIndex().xy;
    uint2 LaunchDimensions = DispatchRaysDimensions().xy;

    float2 d = (((LaunchIndex.xy + 0.5f) / gResolution.xy) * 2.f - 1.f);
    float aspectRatio = (gResolution.x / gResolution.y);

    // Setup the ray
    RayDesc ray;
    ray.Origin = gViewOriginAndTanHalfFovY.xyz;
    ray.Direction = normalize((d.x * gViewMatrix[0].xyz * gViewOriginAndTanHalfFovY.w * aspectRatio) - (d.y * gViewMatrix[1].xyz * gViewOriginAndTanHalfFovY.w) + gViewMatrix[2].xyz);
    ray.TMin = 0.1f;
    ray.TMax = 1000.f;

    // Trace the ray
    HitInfo payload;
    payload.ShadedColorAndHitT = float4(0.f, 0.f, 0.f, 0.f);

    TraceRay(
            gRaytracingAccelerationStructure,
            RAY_FLAG_NONE,
            0xFF,
            0,
            0,
            0,
            ray,
            payload);

    RWTexture2D<float4> output = gRTOutputs[gRenderTargetIdx];
    output[LaunchIndex.xy] = float4(payload.ShadedColorAndHitT.rgb, 1.f);
}

#endif