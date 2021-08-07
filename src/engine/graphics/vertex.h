//
// Created by korona on 2021-06-28.
//

#pragma once

_START_ENGINE
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
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT3 TangentU;
    DirectX::XMFLOAT2 TexCoord;
};

_END_ENGINE
