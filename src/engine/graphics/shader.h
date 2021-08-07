//
// Created by khhan on 2021-06-21.
//


#pragma once

#include "src/engine/graphics/macros.h"

_START_ENGINE
class ShaderVersionedType{
public:
    static inline std::string SHADER_TYPE_VS_5_1 = "vs_5_1";
    static inline std::string SHADER_TYPE_GS_5_1 = "gs_5_1";
    static inline std::string SHADER_TYPE_HS_5_1 = "hs_5_1";
    static inline std::string SHADER_TYPE_DS_5_1 = "ds_5_1";
    static inline std::string SHADER_TYPE_PS_5_1 = "ps_5_1";
};

struct Shader
{
public:
    struct _RenderPass {
    public:
        Microsoft::WRL::ComPtr<ID3DBlob> VS;
        Microsoft::WRL::ComPtr<ID3DBlob> PS;

        bool Build(std::filesystem::path path) {
            if (!Shader::Build(path / "render_pass.hlsl", ShaderVersionedType::SHADER_TYPE_VS_5_1, "VS", VS.GetAddressOf())) {
                return false;
            }
            if (!Shader::Build(path / "render_pass.hlsl", ShaderVersionedType::SHADER_TYPE_PS_5_1, "PS", PS.GetAddressOf())) {
                return false;
            }
            return true;
        }
    };

public:
    inline static _RenderPass RenderPass;

public:
    static bool Build(std::filesystem::path path, std::string type, std::string entry, ID3DBlob** outBlob) {
        HRESULT hr = S_OK;
        unsigned int compile_flag = D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
#if defined(DEBUG) || defined(_DEBUG)
        compile_flag |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        Microsoft::WRL::ComPtr<ID3DBlob> errors;
        Microsoft::WRL::ComPtr<ID3DBlob> shader_byte_code = nullptr;
        hr = ::D3DCompileFromFile(path.c_str(), nullptr,
                                  D3D_COMPILE_STANDARD_FILE_INCLUDE, entry.c_str(),
                                  type.c_str(), compile_flag, 0,
                                  outBlob, &errors
        );
        if (errors != nullptr || FAILED(hr)) {
            if (errors != nullptr)
                GRAPHICS_LOG_ERROR("Shader Build Failed: {}", reinterpret_cast<char *>(errors->GetBufferPointer()));
            return false;
        }
        errors.Reset();
        return true;
    }
};
_END_ENGINE
