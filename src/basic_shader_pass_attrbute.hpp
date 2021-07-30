//
// Created by korona on 2021-07-28.
//

#ifndef KONAI3D_BASIC_SHADER_PASS_ATTRBUTE_HPP
#define KONAI3D_BASIC_SHADER_PASS_ATTRBUTE_HPP

#include "src/engine/graphics/shader_pass.h"
#include "src/macros.h"

_START_KONAI3D
class BasicShaderPassAttribute {
public:
    struct PerObject {
        DirectX::XMFLOAT4X4 world_mat;
    };

    struct PerFrame {
        DirectX::XMFLOAT4X4 view_mat;
        DirectX::XMFLOAT4X4 inverse_view_mat;
        DirectX::XMFLOAT4X4 projection_mat;
    };

public:
    static _ENGINE::ShaderAttribute::ParameterType GetPerFrameAttribute() {
        _ENGINE::ShaderAttribute::ParameterType parameterPerFrame(
                _per_frame, _ENGINE::ShaderParameterVariableType::SHADER_PARAMETER_VARIABLE_TYPE_CB_STATIC
        );

        return parameterPerFrame;
    }

    static _ENGINE::ShaderAttribute::ParameterType GetPerObjectAttribute() {
        _ENGINE::ShaderAttribute::ParameterType parameterPerObject(
                _per_object, _ENGINE::ShaderParameterVariableType::SHADER_PARAMETER_VARIABLE_TYPE_CB_MUTABLE
        );

        return parameterPerObject;
    }

    inline static const std::string _per_frame = "perFrame";
    inline static const std::string _per_object = "perObject";
};
_END_KONAI3D
#endif //KONAI3D_BASIC_SHADER_PASS_ATTRBUTE_HPP
