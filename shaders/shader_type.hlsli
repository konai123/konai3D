#ifndef SHADER_TYPE_HLSLI
#define SHADER_TYPE_HLSLI

#define MaterialType_Lambertian 0
#define MaterialType_Metal      1
#define MaterialType_Dielectric 2

struct Material {
    int DiffuseTextureIndex;
    int MaterialType;
    float Fuzz;
    float RefractIndex;
};

struct RayPayload
{
    float4 HitColor;
    float3 Direction;
    float Pad0;
    float Seed;
    float3 Origin;
    float T;
    float Pdf;

    float3 At() {
        return Origin + T*Direction;
    }

    RayDesc Ray(float tMin, float tMax) {
        const float OriginEpsilon = 0.0001f;
        RayDesc ray;
        ray.Origin = Origin + Direction * OriginEpsilon;
        ray.Direction = Direction;
        ray.TMin = tMin;
        ray.TMax = tMax;
        return ray;
    }
};

struct ShadowRayPayload
{
    bool Visibility;

    RayDesc Ray(float3 origin, float3 target) {
        const float Epsilon = 0.0001f;
        float3 direction = target - origin;

        RayDesc ray;
        ray.Origin = origin;
        ray.Direction = normalize(direction);
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
        return raypay;
    }
};


#define LightType_Point 0
#define LightType_Quad  1
struct Light
{
    int LightType;
    float3 Position;
    float Pad;
    float3 Intensity;

};

#endif