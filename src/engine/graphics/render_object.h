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
    RenderObject(std::shared_ptr<ShaderPass> shaderPass, std::shared_ptr<DrawInfo> drawInfo, std::string name);
    virtual ~RenderObject() = default;

public:
    DirectX::XMMATRIX GetWorldMatrix();
    DirectX::XMVECTOR GetPosition();
    DirectX::XMVECTOR GetRotation();
    DirectX::XMVECTOR GetScale();
    void SetPosition(DirectX::XMVECTOR position);
    void SetRotation(DirectX::XMVECTOR rotation);
    void SetScale(DirectX::XMVECTOR scale);
    std::shared_ptr<DrawInfo> GetDrawInfo();
    std::shared_ptr<ShaderPass> GetShaderPass();
    std::string GetName();

private:
    std::string _name;
    std::shared_ptr<ShaderPass> _shader_pass;
    std::shared_ptr<DrawInfo> _draw_info;

    DirectX::XMFLOAT3 _position;
    DirectX::XMFLOAT3 _rotation;
    DirectX::XMFLOAT3 _scale;
};
_END_ENGINE

#endif //KONAI3D_RENDER_OBJECT_H
