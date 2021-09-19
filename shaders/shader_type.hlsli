#ifndef SHADER_TYPE_HLSLI
#define SHADER_TYPE_HLSLI

#define MaterialType_CookTorrance   0
#define MaterialType_Glass          1
#define MaterialType_Mirror         2

#include <math.hlsli>

struct Material {
    int BaseColorTextureIndex;
    int MaterialType;
    int UseBaseColorTexture;
    float RefractIndex;
    float SpecularPower;
    float Roughness;
    float Metallic;
    float3 EmissiveColor;
    float3 BaseColor;
};

struct RayPayload
{
    float3 L;
    float3 Beta;
    float3 Direction;
    float Seed;
    float3 Origin;
    float T;
    uint CurrDepth;

    float3 At() {
        return Origin + T*Direction;
    }

    RayDesc Ray(float tMin, float tMax) {
        const float OriginEpsilon = 0.0001f;
        RayDesc ray;
        ray.Origin = Origin;
        ray.Direction = Direction;
        ray.TMin = tMin;
        ray.TMax = tMax;
        return ray;
    }
};

struct ShadowRayPayload
{
    float Visibility;

    RayDesc Ray(float3 origin, float3 target) {
        const float Epsilon = 0.0001f;
        float3 direction = target - origin;

        RayDesc ray;
        ray.Direction = normalize(direction);
        ray.Origin = origin;
        ray.TMin = 0.001f;
        ray.TMax = length(direction) - Epsilon;
        return ray;
    }
};

struct Camera
{
    float3 Position;
    float Pad0;
    float3 Direction;
    float Pad1;
    float3 UpVector;
    float Pad2;
    float AspectRatio;
    float Fov;
    float Near;
    float Far;

    RayPayload GetRayPayload(float2 ndc, float seed) {
        //Camera Position
        float FocalLength = 1.0f/tan(Fov * 0.5f);
        float3 r = normalize(cross(UpVector, Direction));
        float3 f = normalize(cross(r, UpVector));
        float3 u = normalize(cross(f, r));
        ndc.x = AspectRatio * ndc.x;

        float3 direction = mul(float3(ndc.xy, FocalLength), float3x3(r, u, f));

        RayPayload raypay;
        raypay.Direction = normalize(direction);
        raypay.Origin = Position;
        raypay.T = 0.f;
        raypay.Seed = seed;
        raypay.L = float3(0.0f, 0.0f, 0.0f);
        raypay.Beta = float3(1.0f, 1.0f, 1.0f);
        raypay.CurrDepth = 0;
        return raypay;
    }
};

#define LightType_Point 0
#define LightType_Quad  1
struct Light
{
    int LightType;
    float3 Position;
    float4x4 Points;
    float3 I;
};

#endif