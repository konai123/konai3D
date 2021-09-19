//
// Created by khhan on 2021-06-21.
//

#pragma once

#include <utility>

#include "src/engine/core/macros.h"
#include "src/engine/core/pool.hpp"
#include "src/engine/graphics/shader_types.h"

_START_ENGINE
class Positionable {
DECLARE_CLASS_AS_INTERFACE(Positionable)
public:
    virtual DirectX::XMMATRIX GetWorldMatrix() = 0;
    virtual void SetTransform (DirectX::FXMMATRIX worldMat) = 0;
    virtual bool IsLight() = 0;
};

class RenderObject : public Positionable {
public:
    RenderObject();
    virtual ~RenderObject() = default;

public:
    static std::optional<RenderObject> AllocRenderObject();
    static void DiscardRenderObject(RenderObject& obj);

public:
    virtual DirectX::XMMATRIX GetWorldMatrix() override;
    virtual void SetTransform (DirectX::FXMMATRIX worldMat) override;
    virtual bool IsLight() override;

public:
    std::string MaterialName;
    UINT ObjectID;
    std::string MeshID;
    UINT SubmeshID;
    DirectX::XMMATRIX WorldMatrix;

public:
    void UpdateMaterial (std::string materialName);
    void UpdateMesh (std::string meshName, UINT submeshID);

private:
    inline static Pool<RenderObject> _pool = Pool<RenderObject>(0, true);

};

struct Light : public Positionable
{
    ShaderType::LightType LightType;
    float3 Position;
    float3 Scale;
    float4 Rotation;
    float4x4 Points;
    float3 I;

public:
    virtual DirectX::XMMATRIX GetWorldMatrix() override {
        auto t = DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z);
        auto s = DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z);
        auto r = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(Rotation.x, Rotation.y, Rotation.z, Rotation.w));
        return s * r * t;
    }

    virtual bool IsLight() override {
        return true;
    }

    virtual void SetTransform (DirectX::FXMMATRIX worldMat) override {
        float4x4 points = {
                -0.5f, 0.5f, 0.0f, 1.0f,
                0.5f, 0.5f, 0.0f, 1.0f,
                -0.5f, -0.5f, 0.0f, 1.0f,
                0.5f, -0.5f, 0.0f, 1.0f,
        };
        auto mat = DirectX::XMLoadFloat4x4(&points);
        auto p = DirectX::XMMatrixMultiply(mat, worldMat);
        DirectX::XMStoreFloat4x4(&Points, p);

        DirectX::XMVECTOR position, scale, rotation;
        DirectX::XMMatrixDecompose(&scale, &rotation, &position, worldMat);
        DirectX::XMStoreFloat3(&Position, position);
        DirectX::XMStoreFloat3(&Scale, scale);
        DirectX::XMStoreFloat4(&Rotation, rotation);
    }
};
_END_ENGINE
