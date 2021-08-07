//
// Created by korona on 2021-08-01.
//

#pragma once

#include "src/engine/core/async_loader.h"

_START_ENGINE
struct Texture {
public:
    UINT width;
    UINT height;
    std::filesystem::path path;
    DirectX::ScratchImage image;
};

class TextureLoader : public _ENGINE::AsyncLoader<Texture> {
public:
    TextureLoader();
    virtual ~TextureLoader() = default;

public:
    virtual void Delegate(std::vector<std::filesystem::path> paths) override;
    std::optional<Texture> LoadTexture(std::filesystem::path path);

};
_END_ENGINE
