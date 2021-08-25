#ifndef DESCRIPTOR_TABLE_HLSLI
#define DESCRIPTOR_TABLE_HLSLI

//Shader Resource
Texture2D gTexture2DTable[] : register(t0, space100);
Texture2DArray gTexture2DArrayTable[] : register(t0, space101);

RWTexture2D<float4> gRTOutputs[]  : register(u0, space100);

#endif