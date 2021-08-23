//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/render_object.h"

_START_ENGINE
RenderObject::RenderObject() {
    WorldMatrix = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f
    };
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

void RenderObject::SetTransform(DirectX::XMMATRIX worldMat) {
    DirectX::XMVECTOR vPosition, vRotation, vScale;
    XMMatrixDecompose(&vScale, &vRotation, &vPosition, worldMat);

    DirectX::XMMATRIX S = DirectX::XMMatrixScalingFromVector(vScale);
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(vRotation);
    DirectX::XMMATRIX T = DirectX::XMMatrixTranslationFromVector(vPosition);

    DirectX::XMMATRIX Mat =  S * R * T;
    XMStoreFloat3x4(reinterpret_cast<float3x4*>(&WorldMatrix), Mat);
}

void RenderObject::UpdateMaterial (std::string materialName) {
    MaterialName = materialName;
}

void RenderObject::UpdateMesh (std::string meshName, UINT submeshID) {
    MeshID = meshName;
    SubmeshID = 0;
}

_END_ENGINE