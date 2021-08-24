#ifndef SHADER_TYPE_HLSL
#define SHADER_TYPE_HLSL

//#pragma pack_matrix( row_major )

struct Material {
    int DiffuseTextureIndex;
};

struct Vertex
{
    float3 PosL;
    float3 Normal;
    float3 TangentU;
    float2 TexCoord;
};
#endif