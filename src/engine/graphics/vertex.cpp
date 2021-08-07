//
// Created by korona on 2021-07-30.
//

#include "vertex.h"

_START_ENGINE
Vertex::Vertex() {}

Vertex::Vertex(
        const DirectX::XMFLOAT3 position,
        const DirectX::XMFLOAT3 normal,
        const DirectX::XMFLOAT3 tangentU,
        const DirectX::XMFLOAT2 texCoord
)
:
        Position(position),
        Normal(normal),
        TangentU(tangentU),
        TexCoord(texCoord) {};

Vertex::Vertex(
        const float px, const float py, const float pz,
        const float nx, const float ny, const float nz,
        const float tx, const float ty, const float tz,
        const float u, const float v
)
:
        Position(px, py, pz),
        Normal(nx, ny, nz),
        TangentU(tx, ty, tz),
        TexCoord(u, v) {};

_END_ENGINE