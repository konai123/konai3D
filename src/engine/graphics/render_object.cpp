//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/render_object.h"

_START_ENGINE
RenderObject::RenderObject() {
    WorldMatrix = DirectX::XMMatrixIdentity();
}

std::optional<RenderObject> RenderObject::AllocRenderObject() {
    int idx = _pool.allocate();
    if (idx < 0) return std::nullopt;

    RenderObject& renderObj = _pool[idx];
    renderObj.ObjectID = idx;
    renderObj.MeshID = "";
    renderObj.MaterialName= "";
    renderObj.SubmeshID = -1;
    return renderObj;
}

void RenderObject::DiscardRenderObject(RenderObject& obj) {
    _pool.free(obj.ObjectID);
}

bool RenderObject::IsLight() {
    return false;
}

void RenderObject::SetTransform(DirectX::FXMMATRIX worldMat) {
    WorldMatrix = worldMat;
}

DirectX::XMMATRIX RenderObject::GetWorldMatrix() {
    return WorldMatrix;
}


void RenderObject::UpdateMaterial (std::string materialName) {
    MaterialName = materialName;
}

void RenderObject::UpdateMesh (std::string meshName, UINT submeshID) {
    MeshID = meshName;
    SubmeshID = 0;
}

_END_ENGINE