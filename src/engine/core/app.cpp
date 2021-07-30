//
// Created by korona on 2021-06-11.
//

#include "src/engine/core/app.h"
#include "src/engine/core/macros.h"

_START_ENGINE
App::App()
:
_input(nullptr),
_width(0),
_height(0),
_min_width(0),
_min_height(0),
_has_quited(false),
_app_name(_T("nonamed")),
_dpi_scale_factor(1.0f)
{
}

bool App::Prepare(HWND hwnd, int width, int height, float dpiFactor) {
    _width = width;
    _height = height;
    _dpi_scale_factor = dpiFactor;

    _input = std::make_unique<Input>();
    if (!_input->Initiate(hwnd)) {
        CORE_LOG_ERROR("Failed to initialize input system.");
        return false;
    }
    return true;
}

void App::OnStart() {
}

void App::OnDestroy() {

}

void App::OnUpdate(float delta) {
}

void App::OnLateUpdate(float delta) {
    _input->Update();
}

void App::OnResizeStart(int width, int height) {
    _width = width;
    _height = height;
}

void App::OnResizeEnd() {
}

void App::OnDPIUpdate(float dpiFactor) {
    _dpi_scale_factor = dpiFactor;
}

void App::Quit() {
    _has_quited = true;
}

tString App::GetAppName() const {
    return _app_name;
}

void App::SetAppName(tString name) {
    _app_name = name;
}

int App::GetWidth() const {
    return _width;
};

int App::GetHeight() const {
    return _height;
};

int App::GetMinWidth() const {
    return _min_width;
}

int App::GetMinHeight() const {
    return _min_height;
}

bool App::HasAppQuited() const {
    return _has_quited;
}

Input *App::GetInput() const {
    return _input.get();
}

void App::SetMinWidth(int width) {
    _min_width = width;
}

void App::SetMinHeight(int height) {
    _min_height = height;
}

float App::GetDPIScaleFactor() {
    return _dpi_scale_factor;
}
_END_ENGINE