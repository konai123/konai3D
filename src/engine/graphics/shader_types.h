//
// Created by korona on 2021-08-03.
//

#pragma once

_START_ENGINE
namespace ShaderType {
struct Material {
public:
    int DiffuseTextureIndex = 0;
};

struct RayPayload
{
    float4 HitColor;
    float3 Direction;
    float Pad0;
    float3 Origin;
    float T;
};

struct Camera
{
    float3 Position;
    float AspectRatio;
    float FocalLength;
    float Fov;
};

}
_END_ENGINE
