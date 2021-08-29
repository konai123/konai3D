#ifndef BSDF_HLSLI
#define BSDF_HLSLI

#include <bxdf.hlsli>
#include <shader_type.hlsli>

struct BSDF
{
    void Init(int matType, float3 baseColor, float fuzzValue)
    {
        albedo = baseColor;
        fuzz = fuzzValue;
        materialType = matType;
    }

    bool Scatter(float3 inDirection, float3 normal, out float3 scatterDirection, out float3 attenuation, inout uint seed)
    {
        if (materialType == MaterialType_Lambertian) {
            Lambertian lam;
            lam.albedo = albedo;
            return lam.Scatter(inDirection, normal, scatterDirection, attenuation, seed);
        }else{
            Metal metal;
            metal.albedo = albedo;
            metal.fuzz = fuzz;
            return metal.Scatter(inDirection, normal, scatterDirection, attenuation, seed);
        }
    }

    float3 albedo;
    float fuzz;
    int materialType;
};

#endif