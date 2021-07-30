//
// Created by khhan on 2021-06-18.
//

#ifndef KONAI3D_VIEWPORT_WINDOW_H
#define KONAI3D_VIEWPORT_WINDOW_H

#include "src/editor/imgui_window.h"
#include "src/camera.h"

_START_KONAI3D
class ViewportWindow : public IMGUIWindow {
public:
    ViewportWindow(std::weak_ptr<_ENGINE::Renderer> renderer);
    virtual ~ViewportWindow() = default;

public:
    virtual void OnUpdate(float delta) override;
    virtual void OnDestroy() override;
    virtual void UpdatePerFrameBuffer();
    virtual void ControlViewport();
    virtual UINT GetWidth() const;
    virtual UINT GetHeight() const;
    virtual std::shared_ptr<_ENGINE::RenderScreen> GetRenderScreen() const;
    virtual std::shared_ptr<Camera> GetCamera() const;

public:
    float _zoom_speed;
    float _drag_speed;

private:
    ImGuiWindowFlags _window_flags = ImGuiWindowFlags_MenuBar;
    ImTextureID _textureID;
    UINT _width;
    UINT _height;
    UINT _fps;
    UINT _frame_cnt;
    float _elapsed_time;
    float _camera_x_angle;
    float _camera_y_angle;
    bool _show_fps_counter;

private:
    std::weak_ptr<_ENGINE::Renderer> _renderer;
    std::shared_ptr<_ENGINE::RenderScreen> _screen;
    std::shared_ptr<Camera> _camera;
};
_END_KONAI3D

#endif //KONAI3D_VIEWPORT_WINDOW_H
