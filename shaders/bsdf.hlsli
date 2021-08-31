#ifndef BSDF_HLSLI
#define BSDF_HLSLI

#include <bxdf.hlsli>
#include <shader_type.hlsli>

struct BSDF
{
    void Init(int matType, float3 baseColor, float fuzzValue, float ir)
    {
        albedo = baseColor;
        fuzz = fuzzValue;
        materialType = matType;
        refractIndex = ir;
    }

    bool Scatter(float3 inDirection, float3 normal, out float3 scatterDirection, out float3 attenuation, out float pdf, inout uint seed)
    {
        if (materialType == MaterialType_Lambertian) {
            Lambertian lam;
            lam.albedo = albedo;
            return lam.Scatter(inDirection, normal, scatterDirection, attenuation, pdf, seed);
        }else if (materialType == MaterialType_Metal) {
            Metal metal;
            metal.albedo = albedo;
            metal.fuzz = fuzz;
            return metal.Scatter(inDirection, normal, scatterDirection, attenuation, pdf, seed);
        }else if (materialType == MaterialType_Dielectric) {
            Dielectric diel;
            diel.ir = refractIndex;
            return diel.Scatter(inDirection, normal, scatterDirection, attenuation, pdf, seed);
        }
        return false;
    }

    float3 albedo;
    float fuzz;
    float refractIndex;
    int materialType;
};

#endif