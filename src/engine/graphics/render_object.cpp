//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/render_object.h"

_START_ENGINE
RenderObject::RenderObject() {
    WorldMatrix = DirectX::XMMatrixIdentity();
}

RenderObject::~RenderObject() {
    _ENGINE::RenderObject::DiscardRenderObject(this);
}

RenderObject *RenderObject::AllocRenderObject() {
    int idx = _pool.allocate();
    if (idx < 0) return nullptr;

    RenderObject& renderObj = _pool[idx];
    renderObj.ObjectID = idx;
    renderObj.MeshID = "";
    renderObj.MaterialName= "";
    renderObj.SubmeshID = -1;
    return &renderObj;
}

void RenderObject::DiscardRenderObject(RenderObject* obj) {
    EngineAssert(obj != nullptr);
    _pool.free(obj->ObjectID);
}

void RenderObject::SetTransform(DirectX::FXMMATRIX worldMat) {
    WorldMatrix = worldMat;
}

void RenderObject::UpdateMaterial (std::string materialName) {
    MaterialName = materialName;
}

void RenderObject::UpdateMesh (std::string meshName, UINT submeshID) {
    MeshID = meshName;
    SubmeshID = 0;
}

_END_ENGINE