//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/render_object.h"

_START_ENGINE
RenderObject::RenderObject() {
    Position = {0.0f, 0.0f, 0.0f};
    Rotation = {0.0f, 0.0f, 0.0f};
    Scale = {1.0f, 1.0f, 1.0f};
    UpdateTransform();
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
    return &renderObj;
}

void RenderObject::DiscardRenderObject(RenderObject* obj) {
    EngineAssert(obj != nullptr);
    _pool.free(obj->ObjectID);
}

void RenderObject::UpdateTransform() {
    DirectX::XMMATRIX ts = {
            Scale.x, 0, 0, 0,
            0, Scale.y, 0, 0,
            0, 0, Scale.z, 0,
            Position.x, Position.y, Position.z, 1
    };

    DirectX::XMMATRIX rot = DirectX::XMMatrixRotationRollPitchYaw(Rotation.x, Rotation.y, Rotation.z);
    DirectX::XMStoreFloat4x4(&WorldMatrix, DirectX::XMMatrixMultiply(ts, rot));
}

void RenderObject::UpdateMaterial (std::string materialName) {
    MaterialName = materialName;
}

void RenderObject::UpdateMesh (std::string meshName) {
    MeshID = meshName;
}

_END_ENGINE