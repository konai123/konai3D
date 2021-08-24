#ifndef DESCRIPTOR_TABLE_HLSL
#define DESCRIPTOR_TABLE_HLSL

//Shader Resource
Texture2D gTexture2DTable[] : register(t0, space100);
Texture2DArray gTexture2DArrayTable[] : register(t0, space101);

#endif