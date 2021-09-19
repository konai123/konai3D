//
// Created by khhan on 2021-06-18.
//

#pragma once

#include "src/editor/imgui_window.h"
#include "src/camera.h"

_START_KONAI3D
class ViewportWindow : public IMGUIWindow {
public:
    ViewportWindow(std::shared_ptr<_ENGINE::Renderer> renderer);
    virtual ~ViewportWindow() = default;

public:
    virtual void OnUpdate(float delta) override;
    virtual void OnDestroy() override;
    virtual void ControlViewport();
    virtual UINT GetWidth() const;
    virtual UINT GetHeight() const;

    virtual void SetResolution(UINT width, UINT height);
    virtual bool VsyncEnabled();
    virtual void SetVSync(bool set);
    virtual _ENGINE::RenderScreen* GetRenderScreen() const;
    virtual Camera* GetCamera() const;
    virtual void ResetCameraAngle();
    virtual void Update();

public:
    UINT GetTraceDepth() const;
    void SetTraceDepth(UINT depth);

private:
    void EditTransform(_ENGINE::Positionable* targetObject);
    void UpdateScreen();

public:
    float ZoomSpeed;
    float DragSpeed;
    _ENGINE::Positionable* SelectedObject;

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
    bool _draw_grid;
    bool _vsync;
    ImGuizmo::OPERATION _guizmo_oper;
    ImGuizmo::MODE _guizmo_mode;

private:
    std::unique_ptr<_ENGINE::RenderScreen> _screen;
    std::shared_ptr<_ENGINE::Renderer> _renderer;
    std::unique_ptr<Camera> _camera;
};
_END_KONAI3D
