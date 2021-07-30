//
// Created by korona on 2021-07-30.
//

#include "src/editor/imgui_window.h"

_START_KONAI3D
IMGUIWindow::IMGUIWindow(std::string name)
:
_window_name (name),
_open(true) {}

void IMGUIWindow::OnUpdate(float delta) {
};

void IMGUIWindow::OnDestroy() {
};

std::string IMGUIWindow::GetWindowName() {
    return _window_name;
}

bool IMGUIWindow::IsCollapsed() {
    return !_open;
}

void IMGUIWindow::Open(bool state) {
    _open = state;
}

void IMGUIWindow::Collapse(bool state) {
    _open = !state;
}

void IMGUIWindow::Name(std::string name) {
    _window_name = name;
}

_END_KONAI3D