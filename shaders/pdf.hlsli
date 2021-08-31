#ifndef PDF_HLSLI
#define PDF_HLSLI

#include <math.hlsli>

struct CosinePDF
{
    float3 GetVector(float3 w, inout uint seed)
    {
        float3x3 onb = GetONB(w);
        float3 direction = normalize(mul(RandomCosineDirection(seed), onb));
        return direction;
    }

    float PDF(float3 w, float3 direction)
    {
        return dot(w, direction) / gPI;
    }
};

#endif