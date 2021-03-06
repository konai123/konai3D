//
// Created by korona on 2021-08-07.
//

#ifndef KONAI3D_MATERIAL_WINDOW_H
#define KONAI3D_MATERIAL_WINDOW_H

#include "src/editor/imgui_window.h"
#include "src/engine/graphics/renderer.h"
#include "src/editor/viewport_window.h"

_START_KONAI3D
class MaterialWindow : public IMGUIWindow {
public:
    MaterialWindow(
            std::shared_ptr<ViewportWindow> viewportWindow,
            std::shared_ptr<_ENGINE::Renderer::ResourceMap> resourceMap
            );

public:
    bool AddMaterial(std::string materialName);

public:
    virtual void OnUpdate(float delta) override;
    virtual void OnDestroy() override;

private:
    ImGui::FileBrowser _file_dialog;
    std::shared_ptr<_ENGINE::Renderer::ResourceMap> _render_resource_map;
    std::shared_ptr<ViewportWindow> _viewport_window;
};

_END_KONAI3D

#endif //KONAI3D_MATERIAL_WINDOW_H
