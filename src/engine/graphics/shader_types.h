//
// Created by korona on 2021-08-03.
//

#pragma once

_START_ENGINE
namespace ShaderType {

enum MaterialType {
    CookTorrance = 0,
    Glass,
    Mirror,
};

struct Material {
    int BaseColorTextureIndex;
    ShaderType::MaterialType MaterialType;
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
    UINT CurrDepth;
};

struct ShadowRayPayload
{
    float Visibility;
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
    LightType_Point = 0,
    LightType_Quad  = 1
};

struct Light
{
    LightType LightType;
    float3 Position;
    float4x4 Points;
    float3 I;
};

}
_END_ENGINE
