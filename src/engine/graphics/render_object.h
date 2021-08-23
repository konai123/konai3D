//
// Created by khhan on 2021-06-21.
//

#pragma once

#include <utility>

#include "src/engine/core/macros.h"
#include "src/engine/core/pool.hpp"

_START_ENGINE
class RenderObject {
public:
    RenderObject();
    virtual ~RenderObject();

public:
    static RenderObject* AllocRenderObject();
    static void DiscardRenderObject(RenderObject* obj);

public:
    float3x4 WorldMatrix;

    std::string MaterialName;
    UINT ObjectID;
    std::string MeshID;
    UINT SubmeshID;

public:
    void SetTransform (DirectX::XMMATRIX worldMat);
    void UpdateMaterial (std::string materialName);
    void UpdateMesh (std::string meshName, UINT submeshID);

private:
    inline static Pool<RenderObject> _pool = Pool<RenderObject>(100, true);

};
_END_ENGINE
