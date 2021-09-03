//
// Created by korona on 2021-06-11.
//

#pragma once

#include "src/engine/core/app.h"
#include "src/engine/graphics/renderer.h"

#include "src/editor/log_window.h"
#include "src/editor/main_window.h"
#include "src/editor/viewport_window.h"
#include "src/editor/component_window.h"
#include "src/editor/material_window.h"

#include "src/imgui_renderer.h"
#include "src/engine/graphics/mesh_map.h"

_START_KONAI3D
class K3DApp : public _ENGINE::App {
public:
    K3DApp();
    virtual ~K3DApp() = default;

public:
    virtual bool Prepare(HWND hwnd, int width, int height, float dpiFactor);
    virtual void OnStart() override;
    virtual void OnDestroy() override;
    virtual void OnUpdate(float delta) override;
    virtual void OnLateUpdate(float delta) override;
    virtual void OnResizeStart(int width, int height) override;
    virtual void OnResizeEnd() override;
    virtual void OnDPIUpdate(float dpiFactor) override;

public:
    static inline std::string DefaultMaterialName = "default";
    static inline std::string DefaultMeshName = "sphere";

private:
    _ENGINE::RenderingOptions _rendering_options;

    std::shared_ptr<_ENGINE::Renderer> _renderer;
    std::shared_ptr<LogWindow> _log_window;
    std::shared_ptr<MainWindow> _main_window;
    std::shared_ptr<ViewportWindow> _viewport_window;
    std::shared_ptr<ComponentWindow> _component_window;
    std::shared_ptr<MaterialWindow> _material_window;
    std::shared_ptr<IMGUIRenderer> _imgui_renderer;

    std::string _ui_font_path;
    UINT _viewport_width;
    UINT _viewport_height;
};
_END_KONAI3D
