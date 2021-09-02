//
// Created by khhan on 2021-09-02.
//

#include "src/asset_manager.h"

_START_KONAI3D
AssetManager& AssetManager::Instance() {
    static AssetManager inst;
    return inst;
}

AssetManager::AssetManager()
{}

bool
AssetManager::Save(std::filesystem::path savePath, ViewportWindow *viewportWindow, _ENGINE::Renderer::ResourceMap* resourceMap) {
    return false;
}

bool
AssetManager::Load(std::filesystem::path loadFile, ViewportWindow *viewportWindow, _ENGINE::Renderer::ResourceMap* resourceMap) {
    return false;
}
_END_KONAI3D