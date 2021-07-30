//
// Created by khhan on 2021-06-18.
//

#ifndef KONAI3D_MAIN_WINDOW_H
#define KONAI3D_MAIN_WINDOW_H

#include "src/editor/imgui_window.h"

_START_KONAI3D
class MainWindow : public IMGUIWindow {
public:
    MainWindow();
    virtual ~MainWindow() = default;

public:
    virtual void OnUpdate(float delta) override;
    virtual void OnDestroy() override;
    virtual bool IsCollapsed() override;
    virtual void Open(bool state) override;

public:
    void AttachWindow(std::shared_ptr<IMGUIWindow> windows);

private:
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
                                    | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
                                    | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                                    | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

private:
    std::vector<std::shared_ptr<IMGUIWindow>> _windows;
};

_END_KONAI3D
#endif //KONAI3D_MAIN_WINDOW_H
