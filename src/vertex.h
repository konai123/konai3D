//
// Created by korona on 2021-06-28.
//

#ifndef KONAI3D_VERTEX_H
#define KONAI3D_VERTEX_H

#include "src/engine/graphics/renderer.h"
#include "src/engine/graphics/render_object.h"

_START_KONAI3D
struct Vertex {
public:
    Vertex();
    Vertex(
            const DirectX::XMFLOAT3 position,
            const DirectX::XMFLOAT3 normal,
            const DirectX::XMFLOAT3 tangentU,
            const DirectX::XMFLOAT2 texCoord
    );
    Vertex(
            const float px, const float py, const float pz,
            const float nx, const float ny, const float nz,
            const float tx, const float ty, const float tz,
            const float u, const float v
    );

public:
    static std::vector<_ENGINE::InputElement> GetInputElements();

public:
    DirectX::XMFLOAT3 _position;
    DirectX::XMFLOAT3 _normal;
    DirectX::XMFLOAT3 _tangent_u;
    DirectX::XMFLOAT2 _tex_coord;
};

_END_KONAI3D
#endif //KONAI3D_VERTEX_H
