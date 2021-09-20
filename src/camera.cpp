//
// Created by korona on 2021-06-30.
//

#include "src/camera.h"
#include "src/math.h"

_START_KONAI3D
Camera::Camera(float fov, float ratio, float pnear, float pfar, float distToFocus, float aperture)
:
        Position(0.0f, 0.0f, 0.0f),
        Direction(0.0f, 0.0f, 1.0f),
        Target(0.0f, 0.0f, 0.0f),
        Fov(fov),
        Ratio(ratio),
        Near(pnear),
        Far(pfar),
        DistToFocus(distToFocus),
        Aperture(aperture),
        CameraUp(0.0f, 1.0f, 0.0f) {}

void Camera::SetPosition(DirectX::XMVECTOR point) {
    DirectX::XMStoreFloat3(&Position, point);
}

void Camera::Transposition(DirectX::XMVECTOR trans) {
    DirectX::XMStoreFloat3(&Position, DirectX::XMVectorAdd(GetCameraPosition(), trans));
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
    DirectX::XMVECTOR up = DirectX::XMVectorSet(CameraUp.x, CameraUp.y, CameraUp.z, 0.0f);
    up = DirectX::XMVector3Transform(up, rot_mat);
    DirectX::XMVECTOR dir = DirectX::XMVector3Transform(GetCameraDirection(), rot_mat);

    DirectX::XMStoreFloat3(&CameraUp, up);
    DirectX::XMStoreFloat3(&Direction, dir);
}

void Camera::LookAt(DirectX::XMVECTOR point, DirectX::XMVECTOR up) {
    DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(point, GetCameraPosition());
    dir = DirectX::XMVector3Normalize(dir);

    DirectX::XMStoreFloat3(&Target, point);
    DirectX::XMStoreFloat3(&CameraUp, up);
    DirectX::XMStoreFloat3(&Direction, dir);
}

DirectX::XMMATRIX Camera::GetViewMatrix() {
    auto up = DirectX::XMVectorSet(CameraUp.x, CameraUp.y, CameraUp.z, 0.0f);
    DirectX::XMVECTOR front = GetCameraDirection();
    return DirectX::XMMatrixLookToLH(GetCameraPosition(), front, up);
}

DirectX::XMMATRIX Camera::GetInverseViewMatrix() {
    auto view_mat = GetViewMatrix();
    auto det = DirectX::XMMatrixDeterminant(view_mat);
    return DirectX::XMMatrixInverse(&det, view_mat);
}

DirectX::XMMATRIX Camera::GetProjectionMatrix() {
    return DirectX::XMMatrixPerspectiveFovLH(Fov, Ratio, Near, Far);
}


DirectX::XMMATRIX Camera::GetViewProjectionMatrix() {
    auto a = GetViewMatrix();
    return DirectX::XMMatrixMultiply(GetViewMatrix(), GetProjectionMatrix());
}

DirectX::XMVECTOR Camera::GetCameraPosition() {
    return DirectX::XMVectorSet(Position.x, Position.y, Position.z, 0.0f);
}

DirectX::XMVECTOR Camera::GetCameraDirection() {
    return DirectX::XMVectorSet(Direction.x, Direction.y, Direction.z, 0.0f);
}

DirectX::XMVECTOR Camera::GetCameraUpVector() {
    return DirectX::XMVectorSet(CameraUp.x, CameraUp.y, CameraUp.z, 0.0f);
}

DirectX::XMVECTOR Camera::GetCameraTarget() {
    return DirectX::XMLoadFloat3(&Target);
}


float Camera::GetCameraDistance() {
    auto target = GetCameraTarget();
    auto position = GetCameraPosition();
    auto dir = DirectX::XMVectorSubtract(target, position);
    return DirectX::XMVector3Length(dir).m128_f32[0];
}

_END_KONAI3D
