//
// Created by korona on 2021-08-03.
//

#pragma once
#include "src/engine/graphics/render_object.h"

_START_ENGINE
namespace ShaderType {

enum MaterialType {
    Lambertian = 0,
    Metal,
    Dielectric
};

struct Material {
public:
    int DiffuseTextureIndex = 0;
    MaterialType MaterialType;
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
    float2 Pad1;
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
    LightType_Point = 0
};

struct Light : public Positionable
{
    LightType LightType;
    float3 Position;

public:
    virtual DirectX::XMMATRIX GetWorldMatrix() {
        return DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z);
    }

    virtual void SetTransform (DirectX::FXMMATRIX worldMat) {
        DirectX::XMVECTOR position, scale, rotation;
        DirectX::XMMatrixDecompose(&scale, &rotation, &position, worldMat);
        DirectX::XMStoreFloat3(&Position, position);
    }
};

}
_END_ENGINE
