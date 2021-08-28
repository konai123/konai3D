#ifndef SHADER_TABLE_HLSLI
#define SHADER_TABLE_HLSLI

#include <vertex.hlsli>

//HitGroup Local Root Args
cbuffer PerObject : register(b0)
{
    uint gMaterialIdx;
};

StructuredBuffer<Vertex> VertexBuffer : register(t2, space0);
StructuredBuffer<uint> IndexBuffer : register(t2, space1);


#endif