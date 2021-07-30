//
// Created by khhan on 2021-06-21.
//

#ifndef KONAI3D_SHADER_H
#define KONAI3D_SHADER_H

_START_ENGINE
enum class ShaderType {
    SHADER_TYPE_VS,
    SHADER_TYPE_GS,
    SHADER_TYPE_HS,
    SHADER_TYPE_DS,
    SHADER_TYPE_PS
};
enum class ShaderParameterVariableType {
    SHADER_PARAMETER_VARIABLE_TYPE_SR_STATIC,
    SHADER_PARAMETER_VARIABLE_TYPE_UA_STATIC,
    SHADER_PARAMETER_VARIABLE_TYPE_CB_STATIC,
    SHADER_PARAMETER_VARIABLE_TYPE_SR_MUTABLE,
    SHADER_PARAMETER_VARIABLE_TYPE_UA_MUTABLE,
    SHADER_PARAMETER_VARIABLE_TYPE_CB_MUTABLE
};

struct ShaderAttribute {
    struct ParameterType {
        ParameterType(std::string name, ShaderParameterVariableType type, D3D12_SHADER_VISIBILITY visibility=D3D12_SHADER_VISIBILITY_ALL)
        :
        name(name),
        type(type),
        visivility(visibility)
        {}
        std::string name;
        ShaderParameterVariableType type;
        D3D12_SHADER_VISIBILITY visivility;
    };


    ParameterType* parameter_types;
    UINT num_parameter_types;
    tString filePath;
    std::string entry_point;
    std::string shader_version;
    ShaderType shader_type;
};

class Shader
{
friend class ShaderPass;
public:
    enum ShaderResourceType {
        SHADER_RESOURCE_CB,
        SHADER_RESOURCE_SR,
        SHADER_RESOURCE_UA
    };

public:
    Shader();
    virtual ~Shader() = default;

public:
    bool Build(ShaderAttribute attribute);
    void Clear();

private:
    ShaderAttribute _attribute;
    std::unordered_map<std::string, ShaderResourceType> _static_resource;
    std::unordered_map<std::string, ShaderResourceType> _mutable_resource;
    std::unordered_map<std::string, D3D12_SHADER_VISIBILITY> _visivility;

    Microsoft::WRL::ComPtr<ID3DBlob> _shader_byte_code;
};
_END_ENGINE

#endif //KONAI3D_SHADER_H
