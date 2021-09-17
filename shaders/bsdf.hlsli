#ifndef BSDF_HLSLI
#define BSDF_HLSLI

#include <bxdf.hlsli>
#include <shader_type.hlsli>

struct LightSample
{
    float3 Wi;
    float3 Position;
    float Pdf;
    float3 Li;
};

LightSample SampleLi(Light light, float3 origin) {
    LightSample sample;
    if (light.LightType == LightType_Point) {
        float l = length(light.Position - origin);
        float l2 = l * l;
        float pdf = 1.0f;
        float3 wi = normalize(light.Position - origin);

        sample.Wi = wi;
        sample.Position = light.Position;
        sample.Pdf = 1.0f;
        sample.Li = light.I / l2;
        return sample;
    }else{
        //Todo
        return sample;
    }
}

struct BSDF
{
    void Init(int matType, float3 baseColor, float3 emissiveColor, float ir, float metallic, float specular, float roughness)
    {
        MaterialType = matType;
        EmissiveColor = emissiveColor;
        IsGlass = false;

        if (MaterialType == MaterialType_Glass) {
            IsGlass = true;
            Glass.ir = ir;
        }else{
            CTorrance.Roughness = roughness;
            CTorrance.BaseColor = baseColor;
            CTorrance.Metallic = metallic;
            CTorrance.SpecularPower = specular;
        }
    }

    BXDFSample Sample(float3 wo, float3 wg, inout uint seed) {
        BXDFSample sample;
        if (MaterialType == MaterialType_Glass) {
            return Glass.Sample(wo, wg, seed);
        }else{
            return CTorrance.Sample(wo, wg, seed);
        }
    }

    float3 F(float3 wi, float3 wg, float3 wo)
    {
        if (MaterialType == MaterialType_Glass) {
            return Glass.Ft(wi, wg, wo);
        }else{
            return CTorrance.Fr(wi, wg, wo);
        }
    }

    float PDF(float3 wi, float3 wg, float3 wo) {
        if (MaterialType == MaterialType_Glass) {
            return Glass.PDF(wi, wg, wo);
        }else{
            return CTorrance.PDF(wi, wg, wo);
        }
    }

    CookTorrance CTorrance;
    Dielectric Glass;

    float3 EmissiveColor;
    int MaterialType;
    bool IsGlass;
};

#endif