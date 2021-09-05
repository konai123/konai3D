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
    float Radius;

public:
    virtual DirectX::XMMATRIX GetWorldMatrix() {
        auto t = DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z);
        auto s = DirectX::XMMatrixScaling(Radius, Radius, Radius);
        return s*t;
    }

    virtual void SetTransform (DirectX::FXMMATRIX worldMat) {
        DirectX::XMVECTOR position, scale, rotation;
        DirectX::XMMatrixDecompose(&scale, &rotation, &position, worldMat);
        DirectX::XMStoreFloat3(&Position, position);
        Radius = scale.m128_f32[0];
    }
};
_END_ENGINE
