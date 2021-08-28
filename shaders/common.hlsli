#ifndef COMMON_HLSLI
#define COMMON_HLSLI

#include "shader_type.hlsli"

//Ray Attributes
struct Attributes
{
	float2 uv;
};

uint GetSeed(uint x, uint y) {
    return 76.897898 * 48.789789 *  cos(x) * sin(y) * 20.79797;
}

#endif