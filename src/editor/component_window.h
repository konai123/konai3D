//
// Created by korona on 2021-07-05.
//

#pragma once

#include "src/editor/imgui_window.h"
#include "src/editor/viewport_window.h"
#include "src/engine/graphics/render_object.h"
#include "src/engine/graphics/mesh_map.h"
#include "src/engine/graphics/material_map.h"

_START_KONAI3D
class ComponentWindow : public IMGUIWindow {
public:
    ComponentWindow(
            std::shared_ptr<ViewportWindow> viewportWindow,
            std::shared_ptr<_ENGINE::Renderer::ResourceMap> resourceMap
    );
    virtual ~ComponentWindow() = default;

public:
    bool AddComponent(std::string name);
    bool DeleteComponent(std::string name);

public:
    virtual void OnUpdate(float delta) override;
    virtual void OnDestroy() override;

private:
    ImGuiWindowFlags _window_flags;
    ImGui::FileBrowser _mesh_file_dialog;
    std::shared_ptr<ViewportWindow> _viewport_window;
    std::shared_ptr<_ENGINE::Renderer::ResourceMap> _render_resource_map;
};
_END_KONAI3D
