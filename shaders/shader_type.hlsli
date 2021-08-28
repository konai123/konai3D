#ifndef SHADER_TYPE_HLSLI
#define SHADER_TYPE_HLSLI

struct Material {
    int DiffuseTextureIndex;
};

struct RayPayload
{
    float4 HitColor;
    float3 Direction;
    float Pad0;
    float Seed;
    float3 Origin;
    float T;
    float3 Pad1;

    float3 At() {
        return Origin + T*Direction;
    }

    RayDesc Ray(float tMin, float tMax) {
        RayDesc ray;
        ray.Origin = Origin;
        ray.Direction = Direction;
        ray.TMin = tMin;
        ray.TMax = tMax;
        return ray;
    }
};

struct Camera
{
    float3 Position;
    float Pad0;
    float AspectRatio;
    float Fov;
    float Near;
    float Far;

    RayPayload GetRayPayload(float2 ndc, float seed) {
        //Camera Position
        float FocalLength = 1.0f/tan(Fov/2.0f);
        float3 direction = float3(ndc.xy, FocalLength) - Position;

        RayPayload raypay;
        raypay.Direction = normalize(direction);
        raypay.Origin = Position;
        raypay.T = 0.f;
        raypay.Seed = seed;

        return raypay;
    }
};
#endif