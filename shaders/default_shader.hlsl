#pragma pack_matrix( row_major )

cbuffer cbPerFrame: register(b0)
{
    float4x4 gViewMat;
    float4x4 gInverseViewMat;
    float4x4 gProjMat;
};

cbuffer cbPerObject : register(b1)
{
    float4x4 gWorldMat;
};

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
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    float4 pos = mul(float4(vin.PosL, 1.0f), gWorldMat);
    pos = mul(pos, gViewMat);
    pos = mul(pos, gProjMat);

    vout.PosH = pos;
    vout.Color = float4(1.0, 0.5, 0.0, 1.0);

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}


