//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/shader.h"
#include "src/engine/graphics/macros.h"

_START_ENGINE
Shader::Shader()
:
_shader_byte_code(nullptr) {
    Clear();
};

bool Shader::Build(ShaderAttribute attribute) {
    /*
     * build shader
     * */
    HRESULT hr = S_OK;
    unsigned int compile_flag = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compile_flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    Microsoft::WRL::ComPtr<ID3DBlob> errors;
    Microsoft::WRL::ComPtr<ID3DBlob> shader_byte_code = nullptr;
    hr = ::D3DCompileFromFile(attribute.filePath.c_str(), nullptr,
                              D3D_COMPILE_STANDARD_FILE_INCLUDE, attribute.entry_point.c_str(),
                              attribute.shader_version.c_str(), compile_flag, 0,
                              _shader_byte_code.GetAddressOf(), &errors
    );
    if (errors != nullptr || FAILED(hr)) {
        if (errors != nullptr)
            GRAPHICS_LOG_ERROR("Shader Build Failed: {}", reinterpret_cast<char *>(errors->GetBufferPointer()));
        return false;
    }
    errors.Reset();

    auto types = attribute.parameter_types;
    using STYPE = ShaderParameterVariableType;
    for (UINT i = 0; i < attribute.num_parameter_types; i++) {
        switch (types[i].type) {
            case STYPE::SHADER_PARAMETER_VARIABLE_TYPE_SR_STATIC:
                _static_resource[types[i].name] = SHADER_RESOURCE_SR;
                break;
            case STYPE::SHADER_PARAMETER_VARIABLE_TYPE_UA_STATIC:
                _static_resource[types[i].name] = SHADER_RESOURCE_UA;
                break;
            case STYPE::SHADER_PARAMETER_VARIABLE_TYPE_CB_STATIC:
                _static_resource[types[i].name] = SHADER_RESOURCE_CB;
                break;
            case STYPE::SHADER_PARAMETER_VARIABLE_TYPE_SR_MUTABLE:
                _mutable_resource[types[i].name] = SHADER_RESOURCE_SR;
                break;
            case STYPE::SHADER_PARAMETER_VARIABLE_TYPE_UA_MUTABLE:
                _mutable_resource[types[i].name] = SHADER_RESOURCE_UA;
                break;
            case STYPE::SHADER_PARAMETER_VARIABLE_TYPE_CB_MUTABLE:
                _mutable_resource[types[i].name] = SHADER_RESOURCE_CB;
                break;
        }
    }
    return true;
}

void Shader::Clear() {
    _static_resource.clear();
    _mutable_resource.clear();
}

_END_ENGINE
