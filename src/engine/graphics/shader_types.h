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
    float Seed;
    float3 Origin;
    float T;
    float3 Pad1;
};

struct Camera
{
    float3 Position;
    float Pad0;
    float AspectRatio;
    float Fov;
    float Near;
    float Far;
};

}
_END_ENGINE
