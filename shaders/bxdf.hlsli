#ifndef BXDF_HLSLI
#define BXDF_HLSLI

#include <math.hlsli>
#include <pdf.hlsli>

struct Lambertian
{
    bool Scatter(float3 inDirection, float3 normal, out float3 scatterDirection, out float3 attenuation, out float pdf, inout uint seed)
    {
        CosinePDF cPDF;
        scatterDirection = cPDF.GetVector(normal, seed);
        pdf = cPDF.PDF(scatterDirection);
        attenuation = albedo;
        return true;
    }

    float Pdf(float3 direction, float3 n) {
        float c = dot(n, normalize(direction));
        if (c <= 0) return 0;
        return c / gPI;
    }

    float3 albedo;
};

struct Metal
{
    bool Scatter(float3 inDirection, float3 normal, out float3 scatterDirection, out float3 attenuation, out float pdf, inout uint seed)
    {
        scatterDirection = normalize(reflect(inDirection, normal) + RandomInUnitSphere(seed) * fuzz);
        attenuation = albedo;
        pdf = 1.0f;
        return (dot(scatterDirection, normal) > 0.0f);
    }

    float3 albedo;
    float3 fuzz;
};

struct Dielectric
{
    bool Scatter(float3 inDirection, float3 normal, out float3 scatterDirection, out float3 attenuation, out float pdf, inout uint seed)
    {
        float ete = 1.0f/ir;
        if (InsideRay(inDirection, normal)) {
            ete = ir;
            normal = -normal;
        }

        float cos_theta = min(dot(-inDirection, normal), 1.0f);
        float sin_theta = sqrt(1.0f - cos_theta * cos_theta);

        if ((ete * sin_theta > 1.0f) || Schlick(cos_theta, ete) > RandomFloat01(seed)) {
            scatterDirection = reflect(inDirection, normal);
        }else{
            scatterDirection = Refract(inDirection, normal, ete);
        }
        attenuation = float3(1.0f, 1.0f, 1.0f);
        pdf = 1.0f;
        return true;
    }

    float Schlick(float cosine, float ref_idx)
    {
        float r0 = (1-ref_idx) / (1+ref_idx);
        r0 = r0*r0;
        return r0 + (1-r0)*pow((1 - cosine),5);
    }

    float ir;
};

#endif