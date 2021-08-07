//
// Created by korona on 2021-08-01.
//

#include "src/engine/graphics/texture_loader.h"
#include "src/engine/graphics/macros.h"

_START_ENGINE
TextureLoader::TextureLoader()
{
}

void TextureLoader::Delegate(std::vector<std::filesystem::path> paths) {
    for (UINT i = 0; i < paths.size(); i++) {
        auto opt = LoadTexture(paths[i]);
        if (opt) {
            Push(std::move(opt.value()));
        }
    }
}

std::optional<Texture> TextureLoader::LoadTexture(std::filesystem::path path) {
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        GRAPHICS_LOG_ERROR("Failed to initialize com");
        return std::nullopt;
    }

    Texture texture;

    auto ext = path.extension().string();

    DirectX::TexMetadata tex_metadata = {};
    DirectX::ScratchImage scratch_image = {};
    if (ext == ".dds") {
        auto hr = DirectX::LoadFromDDSFile(path.c_str(), DirectX::DDS_FLAGS::DDS_FLAGS_FORCE_RGB, &tex_metadata, scratch_image);
        if (FAILED(hr)) {
            GRAPHICS_LOG_ERROR("Failed to load texture images '{}'", path.string());
            return std::nullopt;
        }
    } else if (ext == ".tga") {
        DirectX::ScratchImage from;
        auto hr = DirectX::LoadFromTGAFile(path.c_str(), &tex_metadata, from);
        if (FAILED(hr)) {
            GRAPHICS_LOG_ERROR("Failed to load texture images '{}'", path.string());
            return std::nullopt;
        }
        hr = DirectX::GenerateMipMaps(*from.GetImage(0, 0, 0), DirectX::TEX_FILTER_DEFAULT, 0, scratch_image, false);
        if (FAILED(hr)) {
            GRAPHICS_LOG_ERROR("Failed to generation mipmap textures '{}'", path.string());
            return std::nullopt;
        }
    } else if (ext == ".hdr") {
        DirectX::ScratchImage from;
        auto hr = DirectX::LoadFromHDRFile(path.c_str(), &tex_metadata, from);
        if (FAILED(hr)) {
            GRAPHICS_LOG_ERROR("Failed to load texture images '{}'", path.string());
            return std::nullopt;
        }
        hr = DirectX::GenerateMipMaps(*from.GetImage(0, 0, 0), DirectX::TEX_FILTER_DEFAULT, 0, scratch_image, false);
        if (FAILED(hr)) {
            GRAPHICS_LOG_ERROR("Failed to generation mipmap textures '{}'", path.string());
            return std::nullopt;
        }
    } else {
        DirectX::ScratchImage from;
        auto hr = DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_FORCE_RGB, &tex_metadata, from);

        if (FAILED(hr)) {
            GRAPHICS_LOG_ERROR("Failed to load texture images '{}'", path.string());
            return std::nullopt;
        }
        hr = DirectX::GenerateMipMaps(*from.GetImage(0, 0, 0), DirectX::TEX_FILTER_DEFAULT, 0, scratch_image, false);
        if (FAILED(hr)) {
            GRAPHICS_LOG_ERROR("Failed to generation mipmap textures '{}'", path.string());
            return std::nullopt;
        }
    }
    texture.path = path;
    texture.width= static_cast<UINT>(tex_metadata.width);
    texture.height = static_cast<UINT>(tex_metadata.height);
    texture.image = std::move(scratch_image);

    CoUninitialize();
    return texture;
}

_END_ENGINE