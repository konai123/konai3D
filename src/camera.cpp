//
// Created by korona on 2021-06-30.
//

#include "src/camera.h"
#include "src/math.h"

_START_KONAI3D
Camera::Camera(float fov, float ratio, float pnear, float pfar)
:
_position(0.0f, 0.0f, 0.0f),
_direction(0.0f, 0.0f, 1.0f),
_target(0.0f, 0.0f, 0.0f),
_fov(fov),
_ratio(ratio),
_near(pnear),
_far(pfar),
_camera_up(0.0f, 1.0f, 0.0f) {}

void Camera::SetPosition(DirectX::XMVECTOR point) {
    DirectX::XMStoreFloat3(&_position, point);
}

void Camera::Transposition(DirectX::XMVECTOR trans) {
    DirectX::XMStoreFloat3(&_position, DirectX::XMVectorAdd(GetCameraPosition(), trans));
}

void Camera::MoveOnViewSpace(DirectX::XMVECTOR trans) {
    auto view_mat = GetInverseViewMatrix();
    view_mat.r[3].m128_f32[0] = 0.0f;
    view_mat.r[3].m128_f32[1] = 0.0f;
    view_mat.r[3].m128_f32[2] = 0.0f;
    view_mat.r[3].m128_f32[3] = 0.0f;

    trans = DirectX::XMVector3Transform(trans, view_mat);
    Transposition(trans);
    LookAt(DirectX::XMVectorAdd(GetCameraTarget(), trans), GetCameraUpVector());
}

void Camera::Rotation(DirectX::XMVECTOR eulerAngle) {
    auto rot_mat = DirectX::XMMatrixRotationRollPitchYawFromVector(eulerAngle);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(_camera_up.x, _camera_up.y, _camera_up.z, 0.0f);
    up = DirectX::XMVector3Transform(up, rot_mat);
    DirectX::XMVECTOR dir = DirectX::XMVector3Transform(GetCameraDirection(), rot_mat);

    DirectX::XMStoreFloat3(&_camera_up, up);
    DirectX::XMStoreFloat3(&_direction, dir);
}

void Camera::LookAt(DirectX::XMVECTOR point, DirectX::XMVECTOR up) {
    DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(point, GetCameraPosition());
    dir = DirectX::XMVector3Normalize(dir);

    DirectX::XMStoreFloat3(&_target, point);
    DirectX::XMStoreFloat3(&_camera_up, up);
    DirectX::XMStoreFloat3(&_direction, dir);
}

DirectX::XMMATRIX Camera::GetViewMatrix() {
    auto up = DirectX::XMVectorSet(_camera_up.x, _camera_up.y, _camera_up.z, 0.0f);
    DirectX::XMVECTOR front = GetCameraDirection();
    return DirectX::XMMatrixLookToLH(GetCameraPosition(), front, up);
}

DirectX::XMMATRIX Camera::GetInverseViewMatrix() {
    auto view_mat = GetViewMatrix();
    auto det = DirectX::XMMatrixDeterminant(view_mat);
    return DirectX::XMMatrixInverse(&det, view_mat);
}

DirectX::XMMATRIX Camera::GetProjectionMatrix() {
    return DirectX::XMMatrixPerspectiveFovLH(_fov * F_PI, _ratio, _near, _far);
}


DirectX::XMMATRIX Camera::GetViewProjectionMatrix() {
    auto a = GetViewMatrix();
    return DirectX::XMMatrixMultiply(GetViewMatrix(), GetProjectionMatrix());
}

DirectX::XMVECTOR Camera::GetCameraPosition() {
    return DirectX::XMVectorSet(_position.x, _position.y, _position.z, 0.0f);
}

DirectX::XMVECTOR Camera::GetCameraDirection() {
    return DirectX::XMVectorSet(_direction.x, _direction.y, _direction.z, 0.0f);
}

DirectX::XMVECTOR Camera::GetCameraUpVector() {
    return DirectX::XMVectorSet(_camera_up.x, _camera_up.y, _camera_up.z, 0.0f);
}

DirectX::XMVECTOR Camera::GetCameraTarget() {
    return DirectX::XMLoadFloat3(&_target);
}


float Camera::GetCameraDistance() {
    auto target = GetCameraTarget();
    auto position = GetCameraPosition();
    auto dir = DirectX::XMVectorSubtract(target, position);
    return DirectX::XMVector3Length(dir).m128_f32[0];
}

_END_KONAI3D
