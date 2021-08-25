#ifndef COMMON_HLSLI
#define COMMON_HLSLI

#include "descriptor_table.hlsli"
#include "shader_type.hlsli"
#include "sampler.hlsli"
#include "vertex.hlsli"

cbuffer PerFrame : register(b1, space0)
{
    float4x4 gViewMatrix;
    float4 gViewOriginAndTanHalfFovY;
    float2 gResolution;
    float2 gPad;
    uint gRenderTargetIdx;
};

StructuredBuffer<Material> gMaterials : register(t1, space0);
RaytracingAccelerationStructure gRaytracingAccelerationStructure : register(t1, space1);

struct Attributes 
{
	float2 uv;
};

Vertex GetVertexAttributes(Vertex v0, Vertex v1, Vertex v2, float3 barycentrics)
{
    float3 position = v0.PosL * barycentrics.x + v1.PosL * barycentrics.y + v2.PosL * barycentrics.z;
    float3 normal = v0.Normal * barycentrics.x + v1.Normal * barycentrics.y + v2.Normal * barycentrics.z;
    float3 tangentU = v0.TangentU * barycentrics.x + v1.TangentU * barycentrics.y + v2.TangentU * barycentrics.z;
    float2 texuv = v0.TexCoord * barycentrics.x + v1.TexCoord * barycentrics.y + v2.TexCoord * barycentrics.z;;
    Vertex v;
    v.PosL = position;
    v.Normal = normal;
    v.TangentU = tangentU;
    v.TexCoord = texuv;
	return v;
}

#endif