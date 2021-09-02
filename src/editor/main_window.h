//
// Created by khhan on 2021-06-18.
//

#pragma once

#include "src/editor/imgui_window.h"
#include "src/engine/graphics/renderer.h"

_START_KONAI3D
class MainWindow : public IMGUIWindow {
public:
    MainWindow(
            std::shared_ptr<IMGUIWindow> ViewportWindow,
            std::shared_ptr<IMGUIWindow> ComponentWindow,
            std::shared_ptr<IMGUIWindow> LogWindow,
            std::shared_ptr<IMGUIWindow> MaterialWindow,
            std::shared_ptr<_ENGINE::Renderer::ResourceMap> RenderResourceMap
    );
    virtual ~MainWindow() = default;

public:
    virtual void OnUpdate(float delta) override;
    virtual void OnDestroy() override;
    virtual bool IsCollapsed() override;
    virtual void Open(bool state) override;

private:
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
                                    | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
                                    | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                                    | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

private:
    std::shared_ptr<_ENGINE::Renderer::ResourceMap> _render_resource_map;
    std::vector<std::shared_ptr<IMGUIWindow>> _windows;
    std::shared_ptr<IMGUIWindow> _viewport_windows;
    std::shared_ptr<IMGUIWindow> _component_windows;
    std::shared_ptr<IMGUIWindow> _log_windows;
    std::shared_ptr<IMGUIWindow> _material_windows;

    ImGui::FileBrowser _file_dialog;
};

_END_KONAI3D
