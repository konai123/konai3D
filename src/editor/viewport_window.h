//
// Created by khhan on 2021-06-18.
//

#pragma once

#include "src/editor/imgui_window.h"
#include "src/camera.h"

_START_KONAI3D
class ViewportWindow : public IMGUIWindow {
public:
    ViewportWindow(_ENGINE::Renderer* renderer);
    virtual ~ViewportWindow() = default;

public:
    virtual void OnUpdate(float delta) override;
    virtual void OnDestroy() override;
    virtual void UpdateScreen();
    virtual void ControlViewport();
    virtual UINT GetWidth() const;
    virtual UINT GetHeight() const;
    virtual _ENGINE::RenderScreen* GetRenderScreen() const;
    virtual Camera* GetCamera() const;

private:
    void EditTransform(_ENGINE::RenderObject* targetObject);

public:
    float ZoomSpeed;
    float DragSpeed;
    _ENGINE::RenderObject* SelectedObject;

private:
    ImGuiWindowFlags _window_flags = ImGuiWindowFlags_MenuBar;
    UINT _width;
    UINT _height;
    UINT _fps;
    UINT _frame_cnt;
    float _elapsed_time;
    float _camera_x_angle;
    float _camera_y_angle;
    bool _show_fps_counter;
    ImGuizmo::OPERATION _guizmo_oper;
    ImGuizmo::MODE _guizmo_mode;

private:
    std::unique_ptr<_ENGINE::RenderScreen> _screen;
    std::unique_ptr<Camera> _camera;
};
_END_KONAI3D
