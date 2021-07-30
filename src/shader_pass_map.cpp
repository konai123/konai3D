//
// Created by korona on 2021-07-30.
//

#include "src/shader_pass_map.h"

_START_KONAI3D
ShaderPassMap::ShaderPassMap(_ENGINE::Renderer *renderer) {
//Make Default Shader Pass.
    std::shared_ptr<_ENGINE::Shader> vs = std::make_shared<_ENGINE::Shader>();
    std::shared_ptr<_ENGINE::Shader> ps = std::make_shared<_ENGINE::Shader>();
    _ENGINE::ShaderAttribute attr;
    _ENGINE::ShaderAttribute::ParameterType parameterPerFrame(
            "perFrame", _ENGINE::ShaderParameterVariableType::SHADER_PARAMETER_VARIABLE_TYPE_CB_STATIC
    );
    _ENGINE::ShaderAttribute::ParameterType parameterPerObject(
            "perObject", _ENGINE::ShaderParameterVariableType::SHADER_PARAMETER_VARIABLE_TYPE_CB_MUTABLE
    );
    std::array<_ENGINE::ShaderAttribute::ParameterType, 2> parameters = {
            BasicShaderPassAttribute::GetPerFrameAttribute(),
            BasicShaderPassAttribute::GetPerObjectAttribute()
    };
    attr.parameter_types = parameters.data();
    attr.filePath = (global::ShaderPath / _default_shader_file).wstring();
    attr.entry_point = "VS";
    attr.num_parameter_types = 2;
    attr.shader_type = _ENGINE::ShaderType::SHADER_TYPE_VS;
    attr.shader_version = "vs_5_0";

    if (!vs->Build(attr)) {
        AppAssert(false);
    }

    attr.entry_point = "PS";
    attr.shader_version = "ps_5_0";
    attr.parameter_types = nullptr;
    attr.num_parameter_types = 0;
    if (!ps->Build(attr)) {
        AppAssert(false);
    }
    auto basic_shader_pass = renderer->InstanceShaderPass();
    if (basic_shader_pass == nullptr) {
        AppAssert(false);
    }
    UINT elementsSize = static_cast<UINT>(Vertex::GetInputElements().size());
    auto elements = renderer->InstanceInputElements(Vertex::GetInputElements().data(), elementsSize);
    _ENGINE::ShaderPassAttribute pass_atti;
    if (!basic_shader_pass->Build(
            elements.data(),
            static_cast<UINT>(elements.size()),
            &pass_atti,
            vs,
            ps
    )) {
        AppAssert(false);
    }

    _map[_default_shader_pass] = basic_shader_pass;
}

bool ShaderPassMap::AddShaderPass(std::string name, std::shared_ptr<_ENGINE::ShaderPass> shaderPass) {
    if (_map.contains(name)) {
        APP_LOG_ERROR("'{}' Already has been registered.", name);
        return false;
    }

    _map[name] = shaderPass;
    return true;
}

std::shared_ptr<_ENGINE::ShaderPass> ShaderPassMap::GetShaderPass(std::string name) {
    if (!_map.contains(name)) return nullptr;
    return _map[name];
}

std::vector<std::string> ShaderPassMap::GetShaderPasses() {
    std::vector<std::string> names;
    for (auto &p : _map) {
        names.push_back(p.first);
    }
    return names;
}
_END_KONAI3D