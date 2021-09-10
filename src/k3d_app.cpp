//
// Created by korona on 2021-06-11.
//

#include "src/k3d_app.h"
#include "src/global.h"
#include "src/macros.h"
#include "src/engine/graphics/vertex.h"

_START_KONAI3D
K3DApp::K3DApp()
:
App(),
_renderer(nullptr),
_log_window(nullptr),
_main_window(nullptr),
_ui_font_path(""),
_imgui_renderer(nullptr),
_viewport_width(0),
_viewport_height(0) {
    _rendering_options = {.v_sync = false, .scale_factor = 1.0f};
}

bool K3DApp::Prepare(HWND hwnd, int width, int height, float dpiFactor) {
    if (!App::Prepare(hwnd, width, height, dpiFactor))
        return false;

    SetMinWidth(1920/4);
    SetMinHeight(1080/4);

    _imgui_renderer = std::make_shared<IMGUIRenderer>();
    _renderer = std::make_shared<_ENGINE::Renderer>();
    if (!_renderer->Initiate(
            hwnd, width, height, global::ShaderPath, _imgui_renderer.get()
    )) {
        APP_LOG_ERROR("Failed to initialize render system.");
        return false;
    }

    _log_window = std::make_shared<LogWindow>();

    if (!_log_window->Initiate()) {
        APP_LOG_ERROR("Failed to initialize log window.");
        return false;
    }

    _rendering_options.scale_factor = dpiFactor;
    _renderer->SetRenderingOptions(_rendering_options);

    _viewport_window = std::make_shared<ViewportWindow>(_renderer);
    _component_window = std::make_shared<ComponentWindow>(_viewport_window, _renderer->RenderResourceMap);
    _material_window = std::make_shared<MaterialWindow>(_viewport_window, _renderer->RenderResourceMap);;

    _main_window = std::make_shared<MainWindow>(_viewport_window, _component_window, _log_window, _material_window, _renderer->RenderResourceMap);

    _imgui_renderer->SetEditor(_main_window);
    _ui_font_path = (global::AssetPath / "fonts" / "sansation" / "Sansation_Bold.ttf").string();
    _imgui_renderer->AddFont(_ui_font_path.c_str(), 16);
    _imgui_renderer->SetScale(dpiFactor);

    {
        /* Load System Mesh, Texture */
        auto system_texture_path = global::AssetPath / "textures" / "default_texture.png";
        auto system_mesh_path = global::AssetPath / "meshes" / "sphere.obj";
        DefaultMeshName = system_mesh_path.string();

        _renderer->RenderResourceMap->TextureMap->AsyncLoad({system_texture_path});
        _renderer->RenderResourceMap->MeshMap->AsyncLoad({system_mesh_path});

        _ENGINE::MaterialDesc mat_desc {
            .BaseColorTexturePath = system_texture_path.string(),
            .MaterialType = _ENGINE::ShaderType::Lambertian,
            .Fuzz = 0.f,
            .RefractIndex = 1.5f
        };
        _renderer->RenderResourceMap->MaterialMap->AddMaterial(DefaultMaterialName, mat_desc);
    }
    return true;
}

void K3DApp::OnStart() {
    App::OnStart();
}

void K3DApp::OnDestroy() {
    App::OnDestroy();
    _renderer->OnDestroy(_imgui_renderer.get());
}

void K3DApp::OnUpdate(float delta) {
    App::OnUpdate(delta);
    _ENGINE::RenderScreen* render_target = _viewport_window->GetRenderScreen();
    _renderer->OnRender(delta, render_target, _imgui_renderer.get());
}

void K3DApp::OnLateUpdate(float delta) {
    return App::OnLateUpdate(delta);
}

void K3DApp::OnResizeStart(int width, int height) {
    App::OnResizeStart(width, height);
}

void K3DApp::OnResizeEnd() {
    App::OnResizeEnd();
    _renderer->OnResizeGUI(GetWidth(), GetHeight(), _imgui_renderer.get());
}

void K3DApp::OnDPIUpdate(float dpiFactor) {
    App::OnDPIUpdate(dpiFactor);
    _rendering_options.scale_factor = dpiFactor;
    _renderer->SetRenderingOptions(_rendering_options);
    _imgui_renderer->SetScale(dpiFactor);
}
_END_KONAI3D
