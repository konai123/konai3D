#ifndef SHADER_TYPE_HLSLI
#define SHADER_TYPE_HLSLI

struct Material {
    int DiffuseTextureIndex;
};

struct HitInfo
{
    float4 ShadedColor;
    float HitT;
};
#endif