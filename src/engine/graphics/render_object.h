//
// Created by khhan on 2021-06-21.
//

#pragma once

#include <utility>

#include "src/engine/core/macros.h"
#include "src/engine/core/pool.hpp"

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
_END_ENGINE
