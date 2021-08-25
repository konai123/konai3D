//
// Created by korona on 2021-08-03.
//

#pragma once

_START_ENGINE
namespace ShaderType {
struct Material {
public:
    int DiffuseTextureIndex = 0;
};

struct HitInfo
{
    float4 ShadedColor;
    float HitT;
};

}
_END_ENGINE
