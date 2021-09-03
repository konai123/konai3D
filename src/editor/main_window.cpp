//
// Created by khhan on 2021-06-18.
//

#include "src/editor/main_window.h"
#include "src/asset_manager.h"
#include "src/global.h"
#include "src/macros.h"

_START_KONAI3D
MainWindow::MainWindow(
        std::shared_ptr<IMGUIWindow> ViewportWindow,
        std::shared_ptr<IMGUIWindow> ComponentWindow,
        std::shared_ptr<IMGUIWindow> LogWindow,
        std::shared_ptr<IMGUIWindow> MaterialWindow,
        std::shared_ptr<_ENGINE::Renderer::ResourceMap> RenderResourceMap
)
:
_viewport_windows(ViewportWindow),
_component_windows(ComponentWindow),
_log_windows(LogWindow),
_material_windows(MaterialWindow),
_render_resource_map(RenderResourceMap),
IMGUIWindow("Dockspace")
{
    _windows.resize(4);
    _windows = {_viewport_windows, _component_windows, _log_windows, _material_windows};
    _file_dialog.SetTitle("Load Screen");
}

void MainWindow::OnUpdate(float delta) {
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
        window_flags |= ImGuiWindowFlags_NoBackground;

    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

    /*
     * Top menu bar
     * */
    bool openPopup = false;

    if (ImGui::BeginPopupModal("SaveScreen")) {
        static char buf[20] = {0,};
        ImGui::InputText("SaveName", buf, 20);
        if (ImGui::Button("Ok")) {
            auto path = global::ScreenPath / buf;
            if (!AssetManager::Instance().Save(path,
                                               reinterpret_cast<ViewportWindow *>(_viewport_windows.get()),
                                               _render_resource_map.get())) {
                APP_LOG_ERROR("Failed to save screen {}", buf);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save")) {
                openPopup= true;
            }
            if (ImGui::MenuItem("Load")) {
                _file_dialog.Open();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows")) {
            for (auto p_window : _windows) {
                if (ImGui::MenuItem(p_window->GetWindowName().c_str(), NULL, !p_window->IsCollapsed(), true))
                    p_window->Open(p_window->IsCollapsed());
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (openPopup) {
        ImGui::OpenPopup("SaveScreen");
        openPopup = false;
    }

    /*
     * Render Child windows on main window.
     * */
    {
        for (auto p_window : _windows) {
            if (p_window != nullptr && p_window->IsCollapsed() == false) {
                p_window->OnUpdate(delta);
            }
        }
    }

    _file_dialog.Display();
    if (_file_dialog.HasSelected()) {
        auto selected = _file_dialog.GetSelected();
        APP_LOG_INFO("Load Screen: {}", selected.string());
        _file_dialog.ClearSelected();

        if (!AssetManager::Instance().Load(selected,
                                           reinterpret_cast<ViewportWindow *>(_viewport_windows.get()),
                                           _render_resource_map.get())) {
            APP_LOG_ERROR("Failed to load screen {}", selected.string());
        }
    }

    ImGui::PopStyleVar(5);
    ImGui::End();
}

void MainWindow::OnDestroy() {
    for (auto p_window : _windows) {
        if (p_window != nullptr) {
            p_window->OnDestroy();
        }
    }
}

bool MainWindow::IsCollapsed() {
    return false;
}

void MainWindow::Open(bool state) {
    return;
}
_END_KONAI3D
