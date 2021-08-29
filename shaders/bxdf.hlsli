#ifndef BXDF_HLSLI
#define BXDF_HLSLI

#include <math.hlsli>

struct Lambertian
{
    bool Scatter(float3 inDirection, float3 normal, out float3 scatterDirection, out float3 attenuation, inout uint seed)
    {
        scatterDirection = normal + RandomUnitVector(seed);
        if (NearZero(scatterDirection))
        {
            scatterDirection = normal;
        }
        attenuation = albedo;
        return true;
    }

    float3 albedo;
};

struct Metal
{
    bool Scatter(float3 inDirection, float3 normal, out float3 scatterDirection, out float3 attenuation, inout uint seed)
    {
        scatterDirection = reflect(normalize(inDirection), normal) + RandomInUnitSphere(seed) * fuzz;
        attenuation = albedo;
        return (dot(scatterDirection, normal) > 0.0f);
    }

    float3 albedo;
    float3 fuzz;
};

#endif