//
// Created by khhan on 2021-09-02.
//

#pragma once

#include "src/editor/viewport_window.h"
#include "src/engine/graphics/renderer.h"

_START_KONAI3D
class AssetManager {
public:
    static AssetManager& Instance();

public:
    bool Save (
            std::filesystem::path savePath,
            ViewportWindow* viewportWindow,
            _ENGINE::Renderer::ResourceMap* resourceMap
    );

    bool Load(
            std::filesystem::path loadFile,
            ViewportWindow* viewportWindow,
            _ENGINE::Renderer::ResourceMap* resourceMap
    );

private:
    AssetManager();
};
_END_KONAI3D