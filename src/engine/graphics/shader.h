//
// Created by khhan on 2021-06-21.
//


#pragma once

#include "src/engine/graphics/macros.h"

_START_ENGINE
class ShaderVersionedType{
public:
    static inline tString VS_SHADER_PROFILE_6_3 = L"vs_6_3";
    static inline tString PS_SHADER_PROFILE_6_3 = L"ps_6_3";
};

class LibraryVersionedType{
public:
    static inline tString LIBRARY_PROFILE_6_3 = L"lib_6_3";
};

struct ShaderInfo {
    std::filesystem::path Filename;
    std::filesystem::path IncludeDirectory;
    tString EntryPoint;
    tString TargetProfile;

    ShaderInfo(std::filesystem::path filename, std::filesystem::path includeDirectory, tString entryPoint, tString profile)
    {
        Filename = filename;
        IncludeDirectory = includeDirectory;
        EntryPoint = entryPoint;
        TargetProfile = profile;
    }
};

class ShaderCompiler
{
public:
    static ShaderCompiler& Instance() {
        static ShaderCompiler compiler;
        return compiler;
    }
    ShaderCompiler(ShaderCompiler const&) = delete;
    ShaderCompiler(ShaderCompiler&&) = delete;
    ShaderCompiler& operator=(ShaderCompiler const&) = delete;
    ShaderCompiler& operator=(ShaderCompiler&&) = delete;

protected:
    ShaderCompiler() {
        ThrowHRFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(Compiler.GetAddressOf())));
        ThrowHRFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(Utils.GetAddressOf())));
        ThrowHRFailed(Utils->CreateDefaultIncludeHandler(IncludeHandler.GetAddressOf()));
    }

public:
    Microsoft::WRL::ComPtr<IDxcCompiler3> Compiler = nullptr;
    Microsoft::WRL::ComPtr<IDxcUtils> Utils = nullptr;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> IncludeHandler = nullptr;
};

struct Shader
{
public:
//    struct Rasterize {
//    public:
//        Microsoft::WRL::ComPtr<IDxcBlob> VS;
//        Microsoft::WRL::ComPtr<IDxcBlob> PS;
//
//        bool Build(std::filesystem::path path) {
//            if (!Shader::Build(
//                    {
//                            path / "rasterize_pass.hlsl",
//                            path,
//                            L"VS",
//                            ShaderVersionedType::VS_SHADER_PROFILE_6_3
//                    },
//                    VS.GetAddressOf())) {
//                return false;
//            }
//
//            if (!Shader::Build(
//                    {
//                            path / "rasterize_pass.hlsl",
//                            path,
//                            L"PS",
//                            ShaderVersionedType::PS_SHADER_PROFILE_6_3
//                    },
//                    PS.GetAddressOf())) {
//                return false;
//            }
//            return true;
//        }
//    };

    struct Raytrace {
    public:
        Microsoft::WRL::ComPtr<IDxcBlob> Closest;
        Microsoft::WRL::ComPtr<IDxcBlob> Raygen;
        Microsoft::WRL::ComPtr<IDxcBlob> Miss;
        bool Build(std::filesystem::path path) {
            if (!Shader::Build(
                    {
                            path / "miss.hlsl",
                            path,
                            L"",
                            LibraryVersionedType::LIBRARY_PROFILE_6_3
                    },
                    Miss.GetAddressOf())) {
                return false;
            }

            if (!Shader::Build(
                    {
                            path / "closest.hlsl",
                            path,
                            L"",
                            LibraryVersionedType::LIBRARY_PROFILE_6_3
                    },
                    Closest.GetAddressOf())) {
                return false;
            }

            if (!Shader::Build(
                    {
                            path / "raygen.hlsl",
                            path,
                            L"",
                            LibraryVersionedType::LIBRARY_PROFILE_6_3
                    },
                    Raygen.GetAddressOf())) {
                return false;
            }
            return true;
        }
    };

public:
    inline static Raytrace RaytracePass;

public:
    static bool Build(ShaderInfo info, IDxcBlob** outBlob) {
        LPCWSTR arguments[] =
                {
                    L"-all_resources_bound",
#ifdef _DEBUG
                    L"-WX",				// Warnings as errors
                    L"-Zi",				// Debug info
                    L"-Qembed_debug",	// Embed debug info into the shader
                    L"-Od",				// Disable optimization
#else
                    L"-O3",				// Optimization level 3
#endif
                    // Add include directory
                    L"-I", info.IncludeDirectory.c_str()
                };

        Microsoft::WRL::ComPtr<IDxcCompilerArgs> dxcCompilerArgs;
        ThrowHRFailed(ShaderCompiler::Instance().Utils->BuildArguments(info.Filename.c_str(),
                                                 info.EntryPoint.c_str(),
                                                 info.TargetProfile.c_str(),
                                                 arguments, ARRAYSIZE(arguments),
                                                 nullptr, 0,
                                                 dxcCompilerArgs.ReleaseAndGetAddressOf()));


        Microsoft::WRL::ComPtr<IDxcBlobEncoding> source;
        UINT32 codePage = CP_ACP;

        HRESULT hr = ShaderCompiler::Instance().Utils->LoadFile(info.Filename.c_str(), &codePage, source.ReleaseAndGetAddressOf());
        if (FAILED(hr)) {
            GRAPHICS_LOG_ERROR("Cannot load shader file: {}", info.Filename.string());
            return false;
        }

        BOOL sourceKnown = FALSE;
        UINT sourceCodePage = 0;
        hr = source->GetEncoding(&sourceKnown, &codePage);
        if (FAILED(hr)) {
            GRAPHICS_LOG_ERROR("Cannot get encoding info: {}", info.Filename.string());
            return false;
        }

        DxcBuffer dxcBuffer = {};
        dxcBuffer.Ptr = source->GetBufferPointer();
        dxcBuffer.Size = source->GetBufferSize();
        dxcBuffer.Encoding = sourceCodePage;

        Microsoft::WRL::ComPtr<IDxcResult> dxcResult;
        hr = ShaderCompiler::Instance().Compiler->Compile(&dxcBuffer,
                                             dxcCompilerArgs->GetArguments(), dxcCompilerArgs->GetCount(),
                                             ShaderCompiler::Instance().IncludeHandler.Get(),
                                             IID_PPV_ARGS(dxcResult.ReleaseAndGetAddressOf()));

        dxcResult->GetStatus(&hr);
        if (FAILED(hr))
        {
            IDxcBlobEncoding* error;
            hr = dxcResult->GetErrorBuffer(&error);
            if (FAILED(hr)) {
                GRAPHICS_LOG_ERROR("Cannot assign error buffer");
                return false;
            }

            // Convert error blob to a string
            std::vector<char> infoLog(error->GetBufferSize() + 1);
            memcpy(infoLog.data(), error->GetBufferPointer(), error->GetBufferSize());
            infoLog[error->GetBufferSize()] = 0;

            std::string errorMsg = "Shader Compiler Error:\n";
            errorMsg.append(infoLog.data());

            GRAPHICS_LOG_ERROR("{}", errorMsg);
            return false;
        }
        dxcResult->GetResult(outBlob);
        return true;
    }
};

_END_ENGINE
