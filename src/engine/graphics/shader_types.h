//
// Created by korona on 2021-08-03.
//

#pragma once

_START_ENGINE
namespace ShaderType {

enum MaterialType {
    Lambertian = 0,
    Metal,
    Dielectric,
    Emitter
};

struct Material {
public:
    int BaseColorTextureIndex = 0;
    MaterialType MaterialType;
    float Fuzz;
    float RefractIndex;
    float3 EmittedColor;
    float Pad0;
    float3 Albedo;
    float Pad1;
    int UseBaseColorTexture;
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
    UINT CurrDepth;
};

struct ShadowRayPayload
{
    bool Visibility;
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

};

enum LightType {
    LightType_Sphere = 0
};

struct Light
{
    LightType LightType;
    float3 Position;
    float Radius;
};

}
_END_ENGINE
