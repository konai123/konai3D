#ifndef PATHTRACE_HLSL
#define PATHTRACE_HLSL

#include <common.hlsli>
#include <global_root_args.hlsli>
#include <shader_table.hlsli>
#include <vertex.hlsli>
#include <sampler.hlsli>
#include <math.hlsli>
#include <bsdf.hlsli>

float4 RayColor(RayPayload raypay, uint maxDepth) {
    for (uint i = 0; i < maxDepth; i++) {
        RayDesc r;
        r =  raypay.Ray(0.001f, 100000.0f);

        raypay.CurrDepth = i;
        TraceRay(
                gRaytracingAccelerationStructure,
                RAY_FLAG_NONE,
                0xffffffff,
                0,   /*Hit Gourp Index*/
                1,   /*Ray Stride*/
                0,   /*Miss Shader Index*/
                r,
                raypay);

        if (raypay.T < 0.0f) {
            if (i >= maxDepth - 1) {
                return float4(0.0f, 0.0f, 0.0f, 1.0f);
            }
            break;
        }
    }

    return raypay.HitColor;
}

[shader("raygeneration")]
void RayGen()
{
    uint2 LaunchIndex = DispatchRaysIndex().xy;
    uint2 LaunchDimensions = DispatchRaysDimensions().xy;
    uint seed = uint(LaunchIndex.x * (5555) + LaunchIndex.y * uint(20328) + uint(gTotalFrameCount) * uint(24023));

    const uint sampleCount = 4;

    //Camera Position
    float3 outColor = float3(0.f, 0.f, 0.f);
    for (uint i = 0; i < sampleCount; i++)
    {
        float2 r = float2(RandomFloat01(seed), RandomFloat01(seed));
        float2 uv = (float2(LaunchIndex) + (r - 0.5)) / float2(LaunchDimensions);
        float2 ndc = uv * float2(2,-2) + float2(-1, +1);
        RayPayload raypay = gCamera.GetRayPayload(ndc, seed);
        outColor += RayColor(raypay, 15).xyz;
    }

    outColor.r = isnan(outColor.r) ? 0.0f : outColor.r;
    outColor.g = isnan(outColor.g) ? 0.0f : outColor.g;
    outColor.b = isnan(outColor.b) ? 0.0f : outColor.b;

    RWTexture2D<float4> output = gRTOutputs[gRenderTargetIdx];
    outColor /= float(sampleCount);
    outColor = sqrt(outColor);

    if (gIntegrationCount > 1) {
        outColor = (gIntegrationCount * float3(output[LaunchIndex.xy].xyz) + outColor.xyz) / float(gIntegrationCount+1);
    }

    output[LaunchIndex.xy] = float4(outColor.xyz, 1.0f);
}

bool ShootShadowRay(float3 origin, float3 target)
{
    ShadowRayPayload raypay;
    RayDesc r;
    r =  raypay.Ray(origin, target);

    raypay.Visibility = false;
    const uint flag = RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;
    TraceRay(
            gRaytracingAccelerationStructure,
            flag,
            0xffffffff,
            0,   /*Hit Group Index*/
            1,   /*Ray Stride*/
            1,   /*Miss Shader Index*/
            r,
            raypay);
    return raypay.Visibility;
}

void EstimateLight(float3 origin, inout float3 to, inout uint seed, out float lightPdf)
{
//    if (RandomFloat01(seed) < 0.5f) {
//        int idx = RandomFloat01(seed) * gNumberOfLight+1;
//        Light light = gLights[idx];
//        to = light.ToLight(origin, seed);
//    }

    float lightPdfSum;
    for (uint i = 0; i < gNumberOfLight; i++) {
        lightPdfSum += gLights[i].PDF(origin, to);
    }
    lightPdf = lightPdfSum * (1.0f / float(gNumberOfLight));
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, Attributes attrib)
{
    payload.T = RayTCurrent();

    unsigned int idx0 = IndexBuffer[PrimitiveIndex() * 3 + 0];
    unsigned int idx1 = IndexBuffer[PrimitiveIndex() * 3 + 1];
    unsigned int idx2 = IndexBuffer[PrimitiveIndex() * 3 + 2];

    Vertex vtx0 = VertexBuffer[idx0];
    Vertex vtx1 = VertexBuffer[idx1];
    Vertex vtx2 = VertexBuffer[idx2];

	float3 barycentrics = float3((1.0f - attrib.uv.x - attrib.uv.y), attrib.uv.x, attrib.uv.y);
    Vertex vertex = GetVertexAttributes(vtx0, vtx1, vtx2, barycentrics);

    Material mat = gMaterials[gMaterialIdx];
    int baseColorTextureIndex = mat.BaseColorTextureIndex;
    Texture2D tex = gTexture2DTable[baseColorTextureIndex];
    float3 color = tex.SampleLevel(gSamPointClamp, vertex.TexCoord, 0.0f).rgb;
    float3 worldNormal = normalize(mul(vertex.Normal, (float3x3)WorldToObject3x4()));

    //BSDF Init
    bool isSpecular;
    BSDF bsdf;
    bsdf.Init(mat.MaterialType, color, mat.Fuzz, mat.RefractIndex, mat.EmittedColor, isSpecular);

    float3 outDirection;
    float3 outAttenuation;
    float scatterPdf;
    if (!bsdf.Scatter(payload.Direction, worldNormal, outDirection, outAttenuation, scatterPdf, payload.Seed)) {
        outAttenuation /= payload.Pdf;
        if (payload.CurrDepth == 0) {
            payload.HitColor = float4(outAttenuation.xyz, 1.0f);
        }else{
            payload.HitColor *= float4(outAttenuation.xyz, 1.0f);
        }
        payload.T = -1.0f;
        return;
    }

    outAttenuation /= payload.Pdf;
    if (payload.CurrDepth == 0) {
        payload.HitColor = float4(outAttenuation.xyz, 1.0f);
    }else{
        payload.HitColor *= float4(outAttenuation.xyz, 1.0f);
    }

    if (gNumberOfLight == 0 || isSpecular) {
        payload.Origin = payload.At();
        payload.Direction = normalize(outDirection);
        payload.Pdf = 1.0f;
        return;
    }

    float lightPdf;
    EstimateLight(payload.At(), outDirection, payload.Seed, lightPdf);

    scatterPdf = bsdf.Pdf(outDirection, worldNormal);

    float pdf = 0.5 * lightPdf + 0.5 * scatterPdf;

    payload.Origin = payload.At();
    payload.Direction = normalize(outDirection);
    payload.HitColor *= bsdf.Pdf(outDirection, worldNormal);
    payload.Pdf = pdf;
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    if (gEnvTextureIdx != -1) {
        Texture2D env = gTexture2DTable[gEnvTextureIdx];

        float2 uv = VectorToLatLong(payload.Direction);
        float4 c = env.SampleLevel(gSamLinearClamp, uv, 0.0f);

        c /= payload.Pdf;
        if (payload.CurrDepth == 0) {
            payload.HitColor = float4(c.xyz, 1.0f);
        }else{
            payload.HitColor *= float4(c.xyz, 1.0f);
        }
    }else{
        payload.HitColor *= float4(0.0f ,0.0f, 0.0f, 1.0f);
    }

    payload.T = -1.0f;
}

[shader("miss")]
void ShadowMiss(inout ShadowRayPayload payload)
{
    payload.Visibility = true;
}
#endif