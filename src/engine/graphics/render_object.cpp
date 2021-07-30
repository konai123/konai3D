//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/render_object.h"

_START_ENGINE
RenderObject::RenderObject(std::shared_ptr<ShaderPass> shaderPass, std::shared_ptr<DrawInfo> drawInfo, std::string name)
:
_shader_pass(std::move(shaderPass)),
_draw_info(std::move(drawInfo)),
_name(std::move(name)),
_position({0, 0, 0}),
_rotation({0, 0, 0}),
_scale({1, 1, 1}) {}

DirectX::XMMATRIX RenderObject::GetWorldMatrix() {
    DirectX::XMMATRIX ts = {
            _scale.x, 0, 0, 0,
            0, _scale.y, 0, 0,
            0, 0, _scale.z, 0,
            _position.x, _position.y, _position.z, 1
    };

    DirectX::XMMATRIX rot = DirectX::XMMatrixRotationRollPitchYaw(_rotation.x, _rotation.y, _rotation.z);
    return DirectX::XMMatrixMultiply(ts, rot);
};

DirectX::XMVECTOR RenderObject::GetPosition() {
    return DirectX::XMVectorSet(_position.x, _position.y, _position.z, 1.0f);
}

DirectX::XMVECTOR RenderObject::GetRotation() {
    return DirectX::XMVectorSet(_rotation.x, _rotation.y, _rotation.z, 1.0f);
}

DirectX::XMVECTOR RenderObject::GetScale() {
    return DirectX::XMVectorSet(_scale.x, _scale.y, _scale.z, 1.0f);
}

void RenderObject::SetPosition(DirectX::XMVECTOR position) {
    DirectX::XMStoreFloat3(&_position, position);
}

void RenderObject::SetRotation(DirectX::XMVECTOR rotation) {
    DirectX::XMStoreFloat3(&_rotation, rotation);
}

void RenderObject::SetScale(DirectX::XMVECTOR scale) {
    DirectX::XMStoreFloat3(&_scale, scale);
}

std::shared_ptr<DrawInfo> RenderObject::GetDrawInfo() {
    return _draw_info;
};

std::shared_ptr<ShaderPass> RenderObject::GetShaderPass() {
    return _shader_pass;
}

std::string RenderObject::GetName() {
    return _name;
}

_END_ENGINE