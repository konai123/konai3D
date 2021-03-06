#ifndef GLOBAL_ROOT_ARGS_HLSLI
#define GLOBAL_ROOT_ARGS_HLSLI

#include <shader_type.hlsli>

//Descriptor tables
Texture2D gTexture2DTable[] : register(t0, space100);
Texture2DArray gTexture2DArrayTable[] : register(t0, space101);
RWTexture2D<float4> gRTOutputs[]  : register(u0, space100);

//CBVs
cbuffer PerFrame : register(b1, space0)
{
    Camera gCamera;
    uint gRenderTargetIdx;
    uint gTotalFrameCount;
    uint gIntegrationCount;
    uint gNumberOfLight;
    int gEnvTextureIdx;
    uint gMaxDepth;
};

//SRVs
StructuredBuffer<Material> gMaterials : register(t1, space0);
RaytracingAccelerationStructure gRaytracingAccelerationStructure : register(t1, space1);
StructuredBuffer<Light> gLights : register(t1, space2);


#endif