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
    float3 Origin;
    float T;

    float3 At() {
        return Origin + T*Direction;
    }

    RayDesc Ray() {
        RayDesc ray;
        ray.Origin = Origin;
        ray.Direction = Direction;
        ray.TMin = 0.1f;
        ray.TMax = 1000.f;
        return ray;
    }
};

struct Camera
{
    float3 Position;
    float AspectRatio;
    float FocalLength;
    float Fov;

    RayPayload GetRayPayload(float2 ndc) {
        //Camera Position
        float3 direction = float3(ndc.xy, FocalLength) - Position;

        RayPayload raypay;
        raypay.Direction = normalize(direction);
        raypay.Origin = Position;
        raypay.T = 0.f;

        return raypay;
    }
};
#endif