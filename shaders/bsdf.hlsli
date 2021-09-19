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

LightSample SampleLi(Light light, float3 origin, inout uint seed) {
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
    }else if (light.LightType == LightType_Quad){
        float4x4 points = transpose(light.Points);
        float3 e0 = (points[3] - points[2]).xyz;
        float3 e1 = (points[0] - points[2]).xyz;

        float A = length(cross(e0, e1));

        float X0 = RandomFloat01(seed) - 0.5f;
        float X1 = RandomFloat01(seed) - 0.5f;

        float3 po = light.Position + e0 * X0 + e1 * X1;
        float3 di = origin - po;
        float distance = length(di);

        di = normalize(di);
        float3 N = normalize(cross(e0, e1));

        if (dot(di, N) < 0) {
            N = -N;
        }

        float cosine = dot(N, di);
        float pdf = (distance * distance) / (cosine * A);

        sample.Position = po;
        sample.Pdf = pdf;
        sample.Li = light.I;
        sample.Wi = -di;
        return sample;
    }else{
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
            Glass.ir = ir;
        }else if (MaterialType == MaterialType_CookTorrance){
            CTorrance.Roughness = roughness;
            CTorrance.BaseColor = baseColor;
            CTorrance.Metallic = metallic;
            CTorrance.SpecularPower = specular;
        }

        if (MaterialType == MaterialType_Mirror || MaterialType_Glass) {
            IsGlass = true;
        }
    }

    BXDFSample Sample(float3 wo, float3 wg, inout uint seed) {
        BXDFSample sample;
        if (MaterialType == MaterialType_Glass) {
            return Glass.Sample(wo, wg, seed);
        }else if (MaterialType == MaterialType_CookTorrance) {
            return CTorrance.Sample(wo, wg, seed);
        }else {
            return Mirror.Sample(wo, wg, seed);
        }
    }

    float3 F(float3 wi, float3 wg, float3 wo)
    {
        if (MaterialType == MaterialType_Glass) {
            return Glass.Ft(wi, wg, wo);
        }else if (MaterialType == MaterialType_CookTorrance) {
            return CTorrance.Fr(wi, wg, wo);
        }else {
            return Mirror.Fr(wi, wg, wo);
        }
    }

    float PDF(float3 wi, float3 wg, float3 wo) {
        if (MaterialType == MaterialType_Glass) {
            return Glass.PDF(wi, wg, wo);
        }else if (MaterialType == MaterialType_CookTorrance) {
            return CTorrance.PDF(wi, wg, wo);
        }else{
            return Mirror.PDF(wi, wg, wo);
        }
    }

    CookTorrance CTorrance;
    Dielectric Glass;
    Mirror Mirror;

    float3 EmissiveColor;
    int MaterialType;
    bool IsGlass;
};

#endif