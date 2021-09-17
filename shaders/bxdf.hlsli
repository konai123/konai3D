#ifndef BXDF_HLSLI
#define BXDF_HLSLI

#include <math.hlsli>
#include <pdf.hlsli>

struct BXDFSample
{
    float3 Wi;
    float Pdf;
};

//BTDF
struct Dielectric
{
    float3 Ft(float3 wi, float3 wg, float3 wo) {
        return float3(1.0f, 1.0f, 1.0f);
    }

    BXDFSample Sample(float3 wo, float3 wg, inout uint seed) {
        float ete = 1.0f/ir;
        if (InsideRay(-wo, wg)) {
            ete = ir;
            wg = -wg;
        }

        float cos_theta = min(dot(wo, wg), 1.0f);
        float sin_theta = sqrt(1.0f - cos_theta * cos_theta);

        BXDFSample outSample;
        if ((ete * sin_theta > 1.0f) || FSchlick(cos_theta, ete) > RandomFloat01(seed)) {
            outSample.Wi = reflect(-wo, wg);
        }else{
            outSample.Wi = Refract(-wo, wg, ete);
        }
        outSample.Pdf = 1.0f;
        return outSample;
    }

    float PDF(float3 wi, float3 wg, float3 wo) {
        return 1.0f;
    }

    float FSchlick(float cosine, float ref_idx) {
        float r0 = (1-ref_idx) / (1+ref_idx);
        r0 = r0*r0;
        return r0 + (1-r0)*pow((1 - cosine),5);
    }

    float ir;
};

//BRDF
struct CookTorrance
{
    float3 Fr(float3 wi, float3 wg, float3 wo) {
        float3 SpecularColor = lerp(0.08f*SpecularPower, BaseColor, Metallic);

        float3 wm = normalize(wi+wo);
        float dotIM = dot(wi, wm);
        float dotIG = dot(wi, wg);
        float dotOG = dot(wo, wg);

        float a = max(0.001f, Roughness);
        float a2 = a * a;

        float3 fresnel = FSchlick(SpecularColor, saturate(dotIM));
        float3 specularTerm = GGX(wm, wg) * fresnel * GGXPartialGeometryTerm(wi, wg, wm, a2) * GGXPartialGeometryTerm(wo, wg, wm, a2) / (4.0f*dotIG*dotOG);
        float3 diffuseTerm = dotIG / gPI;

        return (specularTerm + (BaseColor * diffuseTerm * (1-Metallic))) * dotIG;
    }

    BXDFSample Sample(float3 wo, float3 wg, inout uint seed) {
        float SpecularRatio = Metallic;
        BXDFSample outSample;
        float3x3 onb = GetONB(wg);
        if (RandomFloat01(seed) < SpecularRatio) {
            //Specular
            float a = max(0.001f, Roughness);
            float a2 = a * a;

            float e0 = RandomFloat01(seed);
            float e1 = RandomFloat01(seed);

            float theta = acos(sqrt((1.0f - e0) / ((a2 - 1.0f) * e0 + 1.0f)));
            float phi   = g2PI * e1;

            float wmX = sin(theta) * cos(phi);
            float wmY = sin(theta) * sin(phi);
            float wmZ = cos(theta);

            float3 wm = float3(wmX, wmY, wmZ);
            wm = normalize(mul(wm, onb));

            outSample.Wi = 2.0f * dot(wm, wo) * wm - wo;
        }else{
            float3 wi = normalize(mul(RandomCosineDirection(seed), onb));
            outSample.Wi = wi;
        }

        outSample.Pdf = PDF(outSample.Wi, wg, wo);

        return outSample;
    }

    float PDF(float3 wi, float3 wg, float3 wo) {
        //Get Specular PDF
        float specularRatio = Metallic;
        float diffuseRatio = 1.0f - specularRatio;
        float dotGI = dot(wg, wi);
        float3 wm = normalize(wi+wo);

        float specularPdf = 0.0f;
        if (dotGI > 0.0f && dot(wi, wm) > 0.0f) {
            float d = 4 * abs(dot(wo, wm));

            specularPdf = GGX(wm, wg)*dot(wm, wg)/d;
        }

        //Get Diffuse PDF
        float diffusePdf = 0.0f;
        if (dotGI > 0.0f) {
            diffusePdf = dotGI / gPI;
        }
        return specularRatio * specularPdf + diffuseRatio * diffusePdf;
    }

    float X(float v) {
        return v > 0 ? 1 : 0;
    }

    float GGX(float3 wm, float3 wg) {
        float a = max(0.001f, Roughness);
        float a2 = a * a;
        float DotNH = dot(wm, wg);
        float NoH2 = DotNH * DotNH;
        float den = NoH2 * a2 + (1.0f - NoH2);

        return (X(DotNH) * a2) / ( gPI * den * den );
    }

    float3 FSchlick(float3 F0, float radians)
    {
        return F0 + (1-F0) * pow( 1 - radians, 5);
    }

    float GGXPartialGeometryTerm(float3 v, float3 n, float3 h, float a2)
    {
        float VoH2 = dot(v,h);
        float chi = X( VoH2 / dot(v,n));
        VoH2 = VoH2 * VoH2;
        float tan2 = ( 1 - VoH2 ) / VoH2;
        return (chi * 2) / ( 1 + sqrt( 1 + a2 * tan2 ) );
    }

    float Roughness;
    float Metallic;
    float SpecularPower;
    float3 BaseColor;
};

#endif