//
// Created by korona on 2021-07-19.
//

#ifndef KONAI3D_SHADER_PASS_MAP_H
#define KONAI3D_SHADER_PASS_MAP_H

#include "src/engine/graphics/renderer.h"

#include "src/macros.h"
#include "src/global.h"
#include "src/vertex.h"
#include "src/basic_shader_pass_attrbute.hpp"

_START_KONAI3D
class ShaderPassMap {
public:
    ShaderPassMap(_ENGINE::Renderer* renderer);
    virtual ~ShaderPassMap() = default;

public:
    bool AddShaderPass(std::string name, std::shared_ptr<_ENGINE::ShaderPass> shaderPass);
    std::shared_ptr<_ENGINE::ShaderPass> GetShaderPass(std::string name);
    std::vector<std::string> GetShaderPasses();

public:
    inline static const std::string _default_shader_pass = "Only Color";
    inline static const std::string _default_shader_file = "default_shader.hlsl";

private:
    std::unordered_map<std::string, std::shared_ptr<_ENGINE::ShaderPass>> _map;
};
_END_KONAI3D

#endif //KONAI3D_SHADER_PASS_MAP_H
