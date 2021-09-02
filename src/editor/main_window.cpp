//
// Created by khhan on 2021-06-18.
//

#include "src/editor/main_window.h"

_START_KONAI3D
MainWindow::MainWindow()
:
IMGUIWindow("Dockspace") {}

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
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            // Disabling fullscreen would allow the window to be moved to the front of other windows,
            // which we can't undo at the moment without finer window depth/z control.
//            ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
//            ImGui::MenuItem("Padding", NULL, &opt_padding);
            ImGui::MenuItem("Save");
            ImGui::MenuItem("Load");
//            ImGui::Separator();
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

    ImGui::PopStyleVar(5);
    ImGui::End();
}

void MainWindow::OnDestroy() {
    for (int i = 0; i < _windows.size(); i++) {
        _windows[i]->OnDestroy();
    }
}

void MainWindow::AttachWindow(std::shared_ptr<IMGUIWindow> windows) {
    _windows.push_back(windows);
}

bool MainWindow::IsCollapsed() {
    return false;
}

void MainWindow::Open(bool state) {
    return;
}
_END_KONAI3D
