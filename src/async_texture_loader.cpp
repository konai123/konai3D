//
// Created by korona on 2021-08-01.
//

#include "src/async_texture_loader.h"
#include "src/macros.h"

_START_KONAI3D
AsyncTextureLoader::AsyncTextureLoader()
{}

void AsyncTextureLoader::Delegate(std::vector<std::filesystem::path> paths) {
    for (UINT i = 0; i < paths.size(); i++) {
        LoadTexture(paths[i]);
    }
}

void AsyncTextureLoader::LoadTexture(std::filesystem::path path) {
    Texture texture;

    auto ext = path.extension().string();
    auto name = path.filename().string();

    DirectX::TexMetadata tex_metadata = {};
    DirectX::ScratchImage scratch_image = {};
    if (ext == ".dds") {
        auto hr = DirectX::LoadFromDDSFile(path.c_str(), DirectX::DDS_FLAGS::DDS_FLAGS_FORCE_RGB, &tex_metadata, scratch_image);
        if (FAILED(hr)) {
            APP_LOG_ERROR("Failed to load texture images '{}'", path.string());
            return;
        }
    } else if (ext == ".tga") {
        DirectX::ScratchImage from;
        auto hr = DirectX::LoadFromTGAFile(path.c_str(), &tex_metadata, from);
        if (FAILED(hr)) {
            APP_LOG_ERROR("Failed to load texture images '{}'", path.string());
            return;
        }
        hr = DirectX::GenerateMipMaps(*from.GetImage(0, 0, 0), DirectX::TEX_FILTER_DEFAULT, 0, scratch_image, false);
        if (FAILED(hr)) {
            APP_LOG_ERROR("Failed to generation mipmap textures '{}'", path.string());
            return;
        }
    } else if (ext == ".hdr") {
        DirectX::ScratchImage from;
        auto hr = DirectX::LoadFromHDRFile(path.c_str(), &tex_metadata, from);
        if (FAILED(hr)) {
            APP_LOG_ERROR("Failed to load texture images '{}'", path.string());
            return;
        }
        hr = DirectX::GenerateMipMaps(*from.GetImage(0, 0, 0), DirectX::TEX_FILTER_DEFAULT, 0, scratch_image, false);
        if (FAILED(hr)) {
            APP_LOG_ERROR("Failed to generation mipmap textures '{}'", path.string());
            return;
        }
    } else {
        DirectX::ScratchImage from;
        auto hr = DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_FORCE_RGB, &tex_metadata, from);
        if (FAILED(hr)) {
            APP_LOG_ERROR("Failed to load texture images '{}'", path.string());
            return;
        }
        hr = DirectX::GenerateMipMaps(*from.GetImage(0, 0, 0), DirectX::TEX_FILTER_DEFAULT, 0, scratch_image, false);
        if (FAILED(hr)) {
            APP_LOG_ERROR("Failed to generation mipmap textures '{}'", path.string());
            return;
        }
    }
    texture.name = name;
    texture.width= static_cast<UINT>(tex_metadata.width);
    texture.height = static_cast<UINT>(tex_metadata.height);
    texture.image = std::move(scratch_image);

    Push(std::move(texture));
}

_END_KONAI3D