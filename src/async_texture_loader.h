//
// Created by korona on 2021-08-01.
//

#ifndef KONAI3D_ASYNC_TEXTURE_LOADER_H
#define KONAI3D_ASYNC_TEXTURE_LOADER_H

#include "src/engine/core/async_loader.h"

_START_KONAI3D
struct Texture {
public:
    UINT width;
    UINT height;
    std::string name;
    DirectX::ScratchImage image;
};

class AsyncTextureLoader : public _ENGINE::AsyncLoader<Texture> {
public:
    AsyncTextureLoader();
    virtual ~AsyncTextureLoader() = default;

public:
    virtual void Delegate(std::vector<std::filesystem::path> paths) override;

private:
    void LoadTexture(std::filesystem::path path);
};
_END_KONAI3D

#endif //KONAI3D_ASYNC_TEXTURE_LOADER_H
