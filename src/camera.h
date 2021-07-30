//
// Created by korona on 2021-06-30.
//

#ifndef KONAI3D_CAMERA_H
#define KONAI3D_CAMERA_H

_START_KONAI3D
class Camera {
public:
    Camera(
            float fov,
            float ratio,
            float near = 1.0f,
            float far = 1000.0f
    );

    virtual ~Camera() = default;

public:
    virtual void SetPosition(DirectX::XMVECTOR point);
    virtual void Transposition(DirectX::XMVECTOR trans);
    virtual void MoveOnViewSpace(DirectX::XMVECTOR trans);
    virtual void Rotation(DirectX::XMVECTOR eulerAngle);
    virtual void LookAt(DirectX::XMVECTOR point, DirectX::XMVECTOR up);
    virtual DirectX::XMMATRIX GetViewMatrix();
    virtual DirectX::XMMATRIX GetInverseViewMatrix();
    virtual DirectX::XMMATRIX GetProjectionMatrix();
    virtual DirectX::XMMATRIX GetViewProjectionMatrix();
    virtual DirectX::XMVECTOR GetCameraPosition();
    virtual DirectX::XMVECTOR GetCameraDirection();
    virtual DirectX::XMVECTOR GetCameraUpVector();
    virtual DirectX::XMVECTOR GetCameraTarget();
    virtual float GetCameraDistance();

private:
    DirectX::XMFLOAT3 _position;
    DirectX::XMFLOAT3 _direction;
    DirectX::XMFLOAT3 _target;
    DirectX::XMFLOAT3 _camera_up;
    float _fov;
    float _ratio;
    float _near;
    float _far;
};
_END_KONAI3D

#endif //KONAI3D_CAMERA_H
