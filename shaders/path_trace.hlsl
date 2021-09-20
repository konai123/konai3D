#ifndef PATHTRACE_HLSL
#define PATHTRACE_HLSL

#include <common.hlsli>
#include <global_root_args.hlsli>
#include <shader_table.hlsli>
#include <vertex.hlsli>
#include <sampler.hlsli>
#include <math.hlsli>
#include <bsdf.hlsli>

void TerminateRay(inout RayPayload payload) {
    payload.CurrDepth = gMaxDepth;
}

float4 RayColor(RayPayload raypay) {
    while (raypay.CurrDepth < gMaxDepth) {
        RayDesc r;
        r =  raypay.Ray(0.001f, 100000.0f);

        TraceRay(
                gRaytracingAccelerationStructure,
                RAY_FLAG_NONE,
                0xffffffff,
                0,   /*Hit Gourp Index*/
                1,   /*Ray Stride*/
                0,   /*Miss Shader Index*/
                r,
                raypay);

        raypay.CurrDepth++;
    }

    return float4(raypay.L.xyz, 1);
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
        outColor += RayColor(raypay).xyz;
        seed = raypay.Seed;
    }

    outColor.r = isnan(outColor.r) ? 0.0f : outColor.r;
    outColor.g = isnan(outColor.g) ? 0.0f : outColor.g;
    outColor.b = isnan(outColor.b) ? 0.0f : outColor.b;

    RWTexture2D<float4> output = gRTOutputs[gRenderTargetIdx];
    outColor /= float(sampleCount);

    if (gIntegrationCount > 1) {
        outColor = (gIntegrationCount * float3(output[LaunchIndex.xy].xyz) + outColor.xyz) / float(gIntegrationCount+1);
    }

    output[LaunchIndex.xy] = float4(outColor.xyz, 1.0f);
}

float ShootShadowRay(float3 origin, float3 target)
{
    ShadowRayPayload raypay;
    RayDesc r;
    r =  raypay.Ray(origin, target);

    raypay.Visibility = 0;
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

//Direct Lighting
float3 EstimateDirection(Light light, float3 origin, float3 wg, float3 wo, BSDF bsdf, inout uint seed)
{
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    LightSample sample = SampleLi(light, origin, seed);
    if (sample.Pdf > 0.0f && any(sample.Li)) {
        float vis = ShootShadowRay(origin + wg * 0.0001f, sample.Position);
        float mixPDF = (sample.Pdf * 0.5 + bsdf.PDF(sample.Wi, wg, wo) * 0.5);
        Lo += bsdf.F(sample.Wi, wg, wo) * sample.Li * vis / mixPDF;
    }

    return Lo;
}

float3 SampleOneLight(float3 origin, float3 wg, float3 wo, BSDF bsdf, inout uint seed)
{
    if (gNumberOfLight == 0) {
        return float3(0.0f, 0.0f, 0.0f);
    }
    float lightPdf = 1.0f / float(gNumberOfLight);
    int lightIndex = RandomFloat01(seed) * gNumberOfLight;
    Light light = gLights[lightIndex];
    return EstimateDirection(light, origin, wg, wo, bsdf, seed) / lightPdf;
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload : SV_RayPayload, in Attributes attrib)
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
    float3 worldNormal = normalize(mul(vertex.Normal, (float3x3)WorldToObject3x4()));

    Material mat = gMaterials[gMaterialIdx];
    float3 color;
    if (mat.UseBaseColorTexture != 0) {
        int baseColorTextureIndex = mat.BaseColorTextureIndex;
        Texture2D tex = gTexture2DTable[baseColorTextureIndex];
        color = tex.SampleLevel(gSamLinearWrap, vertex.TexCoord, 0.0f).rgb;
    }else {
        color = mat.BaseColor;
    }

    //BSDF Init
    BSDF bsdf;
    bsdf.Init(mat.MaterialType, color, mat.EmissiveColor, mat.RefractIndex, mat.Metallic, mat.SpecularPower, mat.Roughness);

    float3 origin = payload.At();
    float3 wg = worldNormal;
    float3 wo = -payload.Direction;

    if (!InsideRay(wg, wo)) {
        wg = -wg;
    }

    if (!bsdf.IsGlass) {
        payload.L += payload.Beta * SampleOneLight(origin, wg, wo, bsdf, payload.Seed);
    }

    BXDFSample sample = bsdf.Sample(wo, wg, payload.Seed);
    float3 wi = sample.Wi;

    payload.L += bsdf.EmissiveColor;
    if (sample.Pdf < gEps) {
        TerminateRay(payload);
    }else{
        payload.Beta *= bsdf.F(wi, wg, wo) / sample.Pdf;
    }

    payload.Direction = wi;
    payload.Origin = origin;

    //Russian roulette
    float3 rr = payload.Beta;
    float maxCmp = max(rr.x, max(rr.y, rr.z));
    if (maxCmp < 1.0f && payload.CurrDepth > 1) {
        float q = max(0.0f, 1.0f - maxCmp);
        if (RandomFloat01(payload.Seed) < q) {
            TerminateRay(payload);
        }
        payload.Beta /= 1.0f - q;
    }
    return;
}

[shader("miss")]
void Miss(inout RayPayload payload : SV_RayPayload)
{
    if (gEnvTextureIdx != -1) {
        Texture2D env = gTexture2DTable[gEnvTextureIdx];

        float2 uv = VectorToLatLong(payload.Direction);
        float4 c = env.SampleLevel(gSamLinearClamp, uv, 0.0f);
        payload.L += payload.Beta * float3(c.xyz);
    }

    TerminateRay(payload);
}

[shader("miss")]
void ShadowMiss(inout ShadowRayPayload payload : SV_RayPayload)
{
    payload.Visibility = 1.0f;
}
#endif