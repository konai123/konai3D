//
// Created by khhan on 2021-06-21.
//

#ifndef KONAI3D_RENDER_OBJECT_H
#define KONAI3D_RENDER_OBJECT_H

#include <utility>

#include "src/engine/core/macros.h"
#include "src/engine/graphics/shader_pass.h"
#include "src/engine/graphics/vertex_buffer.h"
#include "src/engine/graphics/rendered.h"

_START_ENGINE
class RenderObject : public Rendered {
public:
    RenderObject(std::string shaderPassName, std::string drawInfoID, std::string name);
    virtual ~RenderObject() = default;

public:
    DirectX::XMMATRIX GetWorldMatrix();
    DirectX::XMVECTOR GetPosition();
    DirectX::XMVECTOR GetRotation();
    DirectX::XMVECTOR GetScale();
    void SetPosition(DirectX::XMVECTOR position);
    void SetRotation(DirectX::XMVECTOR rotation);
    void SetScale(DirectX::XMVECTOR scale);

    void SetShaderPassName(std::string name);
    void SetDrawInfoID(std::string name);

    std::string GetShaderPassName();
    std::string GetDrawInfoID();
    std::string GetName();

private:
    std::string _name;
    std::string _draw_info_id;
    std::string _texture_name;
    std::string _shader_pass_name;

    DirectX::XMFLOAT3 _position;
    DirectX::XMFLOAT3 _rotation;
    DirectX::XMFLOAT3 _scale;
};
_END_ENGINE

#endif //KONAI3D_RENDER_OBJECT_H
