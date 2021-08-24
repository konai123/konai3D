#ifndef RENDER_PASS_HLSL
#define RENDER_PASS_HLSL

#include "shader_type.hlsli"
#include "descriptor_table.hlsli"
#include "sampler.hlsl"

struct VertexIn
{
    float3 PosL  : POSITION;
    float3 Normal : NORMAL;
    float3 TangentU : TANGENTU;
    float2 TexCoord : TEXCOORD;
};

struct VertexOut
{
    float4 PosH  : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

cbuffer gCBPerObject : register (b0, space0)
{
    float4x4 gWorldMat;
    int gMaterialID;
};

cbuffer gCBPerFrame : register (b1, space0)
{
    float4x4 gViewMat;
    float4x4 gInverseViewMat;
    float4x4 gProjectionMat;
};

StructuredBuffer<VertexIn> gVertices : register(t1, space0);
StructuredBuffer<uint> gIndices: register(t1, space1);

VertexOut VS(in uint ibID : SV_VertexID)
{
    VertexOut vout;
    VertexIn vin;

    uint vbID = gIndices[ibID];
    vin = gVertices[vbID];
    float4 pos = mul(float4(vin.PosL, 1.0f), gWorldMat);
    pos = mul(pos, gViewMat);
    pos = mul(pos, gProjectionMat);

    vout.PosH = pos;
    vout.TexCoord = vin.TexCoord;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    Material mat = gMaterials[gMaterialID];
    Texture2D tex = gTexture2DTable[mat.DiffuseTextureIndex];
    float4 diffuseColor = tex.Sample(gsamAnisotropicWrap, pin.TexCoord);

    return diffuseColor;
}

#endif