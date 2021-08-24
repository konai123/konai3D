#ifndef CLOSEST_HLSL
#define CLOSEST_HLSL

#include "common.hlsli"

cbuffer PerObject : register(b0)
{
	uint gMaterialIdx;
};

StructuredBuffer<Vertex> VertexBuffer : register(t2, space0);
StructuredBuffer<uint> IndexBuffer : register(t2, space1);

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

	payload.ShadedColorAndHitT = float4(1.0f, 0.0f, 0.0f, RayTCurrent());
}

#endif