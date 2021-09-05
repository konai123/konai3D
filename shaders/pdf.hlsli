#ifndef PDF_HLSLI
#define PDF_HLSLI

#include <math.hlsli>

struct CosinePDF
{
    float3 GetVector(float3 w, inout uint seed)
    {
        onb = GetONB(w);
        currNormal = w;
        float3 direction = normalize(mul(RandomCosineDirection(seed), onb));
        return direction;
    }

    float PDF(float3 direction)
    {
        float c = dot(currNormal, normalize(direction));
        if (c <= 0) return 0;
        return c / gPI;
    }

    float3 currNormal;
    float3x3 onb;
};

#endif