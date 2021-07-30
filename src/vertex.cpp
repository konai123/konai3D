//
// Created by korona on 2021-07-30.
//

#include "src/vertex.h"

_START_KONAI3D
Vertex::Vertex() {}

Vertex::Vertex(
        const DirectX::XMFLOAT3 position,
        const DirectX::XMFLOAT3 normal,
        const DirectX::XMFLOAT3 tangentU,
        const DirectX::XMFLOAT2 texCoord
)
:
_position(position),
_normal(normal),
_tangent_u(tangentU),
_tex_coord(texCoord) {};

Vertex::Vertex(
        const float px, const float py, const float pz,
        const float nx, const float ny, const float nz,
        const float tx, const float ty, const float tz,
        const float u, const float v
)
:
_position(px, py, pz),
_normal(nx, ny, nz),
_tangent_u(tx, ty, tz),
_tex_coord(u, v) {};

std::vector<_ENGINE::InputElement> Vertex::GetInputElements() {
    std::vector<_ENGINE::InputElement> elements =
    {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0},
            {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 12},
            {"TANGENTU", 0, DXGI_FORMAT_R32G32B32_FLOAT, 24},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    36},
    };

    return elements;
}

_END_KONAI3D