//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/render_object.h"

_START_ENGINE
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



//DirectX::XMMATRIX RenderObject::UpdateWorldMatrix() {
//    DirectX::XMMATRIX ts = {
//            _scale.x, 0, 0, 0,
//            0, _scale.y, 0, 0,
//            0, 0, _scale.z, 0,
//            _position.x, _position.y, _position.z, 1
//    };
//
//    DirectX::XMMATRIX rot = DirectX::XMMatrixRotationRollPitchYaw(_rotation.x, _rotation.y, _rotation.z);
//    return DirectX::XMMatrixMultiply(ts, rot);
//};
//
//DirectX::XMVECTOR RenderObject::GetPosition() {
//    return DirectX::XMVectorSet(_position.x, _position.y, _position.z, 1.0f);
//}
//
//DirectX::XMVECTOR RenderObject::GetRotation() {
//    return DirectX::XMVectorSet(_rotation.x, _rotation.y, _rotation.z, 1.0f);
//}
//
//DirectX::XMVECTOR RenderObject::GetScale() {
//    return DirectX::XMVectorSet(_scale.x, _scale.y, _scale.z, 1.0f);
//}

_END_ENGINE