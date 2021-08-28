#ifndef PATHTRACE_HLSL
#define PATHTRACE_HLSL

#include <common.hlsli>
#include <global_root_args.hlsli>
#include <shader_table.hlsli>
#include <vertex.hlsli>
#include <sampler.hlsli>
#include <math.hlsli>

float4 RayColor(RayPayload raypay, uint maxDepth) {
    float4 outColor = float4(1.0f, 1.0f, 1.0f, 1.0f);

    for (uint i = 0; i < maxDepth; i++) {
        RayDesc r;
        r =  raypay.Ray(0.0001f, 100000.0f);
        TraceRay(
                gRaytracingAccelerationStructure,
                RAY_FLAG_NONE,
                0xffffffff,
                0,   /*Hit Gourp Index*/
                1,   /*Ray Stride*/
                0,   /*Miss Shader Index*/
                r,
                raypay);

        outColor *= 0.5f;
        if (raypay.T < 0.0f) {
            outColor *= raypay.HitColor;
            break;
        }
    }

    return float4(outColor.xyz, 1.0f);
}

[shader("raygeneration")]
void RayGen()
{
    uint2 LaunchIndex = DispatchRaysIndex().xy;
    uint2 LaunchDimensions = DispatchRaysDimensions().xy;
    uint seed = GetSeed(LaunchIndex.x, LaunchIndex.y);
    const uint sampleCount = 30;

    //Camera Position
    float4 outColor = float4(0.f, 0.f, 0.f, 0.f);
    for (uint i = 0; i < sampleCount; i++)
    {
        float2 r = float2(RandomFloat01(seed), RandomFloat01(seed));
        float2 uv = (float2(LaunchIndex) + (r - 0.5)) / float2(LaunchDimensions);
        float2 ndc = uv * float2(2,-2) + float2(-1, +1);
        RayPayload raypay = gCamera.GetRayPayload(ndc, seed);
        outColor += RayColor(raypay, 30);
    }

    RWTexture2D<float4> output = gRTOutputs[gRenderTargetIdx];
    outColor /= float(sampleCount);

    output[LaunchIndex.xy] = float4(sqrt(outColor.xyz), 1.0f);
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
    int diffuseTextureIndex = mat.DiffuseTextureIndex;
    Texture2D diffuse = gTexture2DTable[diffuseTextureIndex];
    float3 color = diffuse.SampleLevel(gSamPointClamp, vertex.TexCoord, 0.0f).rgb;

    uint seed = payload.Seed;
    float3 worldNormal = normalize(mul(vertex.Normal, (float3x3)WorldToObject3x4()));

    payload.Origin = payload.At();
    payload.Direction = normalize(payload.Origin + worldNormal + normalize(RandomInUnitSphere(seed)));
	payload.HitColor = float4((vertex.Normal.xyz+1.f)*0.5, 1.0f);
	payload.Seed = seed;
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    float t = 0.5f*(WorldRayDirection().y + 1.0f);
    payload.HitColor = (1.0f-t)*float4(1.0f, 1.0f, 1.0f, 1.0f) + t*float4(0.5f, 0.7f, 1.0f, 1.0f);
    payload.T = -1.0f;
}
#endif