#ifndef PATHTRACE_HLSL
#define PATHTRACE_HLSL

#include <common.hlsli>
#include <global_root_args.hlsli>
#include <shader_table.hlsli>
#include <vertex.hlsli>
#include <sampler.hlsli>
#include <math.hlsli>

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

[shader("closesthit")]
void ClosestHit(inout HitInfo payload, Attributes attrib)
{
    unsigned int idx0 = IndexBuffer[PrimitiveIndex() * 3 + 0];
    unsigned int idx1 = IndexBuffer[PrimitiveIndex() * 3 + 1];
    unsigned int idx2 = IndexBuffer[PrimitiveIndex() * 3 + 2];

    Vertex vtx0 = VertexBuffer[idx0];
    Vertex vtx1 = VertexBuffer[idx1];
    Vertex vtx2 = VertexBuffer[idx2];

	float3 barycentrics = float3((1.0f - attrib.uv.x - attrib.uv.y), attrib.uv.x, attrib.uv.y);
    Vertex vertex = GetVertexAttributes(vtx0, vtx1, vtx2, barycentrics);

    Material mat = gMaterials[gMaterialIdx];
    int diffuseTextureIndex = mat.DiffuseTextureIndex;
    Texture2D diffuse = gTexture2DTable[diffuseTextureIndex];
    float3 color = diffuse.SampleLevel(gSamPointClamp, vertex.TexCoord, 0.0f).rgb;

	payload.ShadedColor = float4((vertex.Normal.xyz+1.f)*0.5, 1.0f);
	payload.HitT = RayTCurrent();
}

[shader("miss")]
void Miss(inout HitInfo payload)
{
    float t = 0.5f*(WorldRayDirection().y + 1.0f);
    payload.ShadedColor = (1.0f-t)*float4(1.0f, 1.0f, 1.0f, 1.0f) + t*float4(0.5f, 0.7f, 1.0f, 1.0f);
    payload.HitT = -1.0f;
}
#endif