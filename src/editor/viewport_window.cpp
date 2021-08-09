//
// Created by khhan on 2021-06-18.
//

#include "src/editor/viewport_window.h"

#include <utility>
#include "src/macros.h"
#include "src/math.h"

_START_KONAI3D
ViewportWindow::ViewportWindow(_ENGINE::Renderer *renderer)
:
IMGUIWindow("Viewport"),
_width(1920),
_height(1080),
_fps(0),
_frame_cnt(0),
_elapsed_time(0.0f),
_show_fps_counter(false),
ZoomSpeed(1.0f),
_camera_x_angle(0.0f),
_camera_y_angle(0.0f),
_screen(nullptr) {
    _camera = std::make_shared<Camera>(
        0.25f,
        static_cast<float>(_width) / static_cast<float>(_height),
        0.1f,
        1000.0f
    );

    _camera->SetPosition(DirectX::XMVectorSet(0.0f, 0.0f, -5.0f, 1.0f));
    _camera->LookAt(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

    DragSpeed = 1.0f / static_cast<float>(_width);

    _screen = renderer->InstanceRenderScreen(_width, _height);
    AppAssert(_screen != nullptr);

    DirectX::XMStoreFloat4x4(&_screen->ViewMatrix, _camera->GetViewMatrix());
    DirectX::XMStoreFloat4x4(&_screen->ProjectionMatrix, _camera->GetProjectionMatrix());
    DirectX::XMStoreFloat4x4(&_screen->InverseViewMatrix, _camera->GetInverseViewMatrix());
}

void ViewportWindow::OnUpdate(float delta) {
    std::string name = GetWindowName();
    if (!ImGui::Begin(name.c_str(), &_open, _window_flags)) {
        ImGui::End();
        return;
    }

    if (ImGui::IsWindowHovered())
    {
        ControlViewport();
    }

    ImVec2 size = ImGui::GetContentRegionAvail();
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    ImVec2 window_pos = ImGui::GetWindowPos();

    _elapsed_time += delta;
    _frame_cnt++;
    if (_elapsed_time > 1.0f) {
        _elapsed_time = 0.0f;
        _fps = _frame_cnt;
        _frame_cnt = 0;
    }

    ImVec2 menu_size = {0, 0};
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Show")) {
            if (ImGui::MenuItem("Show FPS Counter", NULL, _show_fps_counter, true)) {
                _show_fps_counter = !_show_fps_counter;
            }
            ImGui::EndMenu();
        }
        menu_size = ImGui::GetItemRectSize();
        ImGui::EndMenuBar();
    }

    if (_show_fps_counter) {
        const ImVec2 p0 = ImGui::GetItemRectMin();
        const ImVec2 text_pos = {p0.x + 2.0f, p0.y + menu_size.y + 2.0f};
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        std::string fps = fmt::format("FPS: {}, DeltaTime: {}", _fps, delta);
        draw_list->AddText(text_pos, IM_COL32_WHITE, fps.c_str());
    }
    static const float identityMatrix[16] =
            { 1.f, 0.f, 0.f, 0.f,
              0.f, 1.f, 0.f, 0.f,
              0.f, 0.f, 1.f, 0.f,
              0.f, 0.f, 0.f, 1.f };

    UpdateScreen();
    ImGuizmo::SetDrawlist();
    ImGuizmo::DrawGrid(
            reinterpret_cast<float*>(&_screen->ViewMatrix),
            reinterpret_cast<float*>(&_screen->ProjectionMatrix),
            identityMatrix,
            100.0f);

    ImGuizmo::SetRect(cursor_pos.x, cursor_pos.y, size.x, size.y);

    ImGui::Image(reinterpret_cast<void *>(_screen->GetShaderResourceHeapDesc()->GpuHandle.ptr), size);
    ImGui::End();
}

void ViewportWindow::OnDestroy() {

}

UINT ViewportWindow::GetWidth() const {
    return _width;
}

UINT ViewportWindow::GetHeight() const {
    return _height;
}

std::shared_ptr<Camera> ViewportWindow::GetCamera() const {
    return _camera;
}

void ViewportWindow::ControlViewport() {
    float m_wheel_delta = ImGui::GetIO().MouseWheel;
    EngineAssert(_camera != nullptr);
    if (m_wheel_delta != 0) {
        float distance = _camera->GetCameraDistance();
        distance -= m_wheel_delta * ZoomSpeed;
        if (distance <= 1.0f) {
            distance = 1.0f;
        }
        DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(_camera->GetCameraPosition(), _camera->GetCameraTarget());
        dir = DirectX::XMVector3Normalize(dir);

        auto target = _camera->GetCameraTarget();
        dir = DirectX::XMVectorMultiply(dir, DirectX::XMVectorSet(distance, distance, distance, 0.0f));
        auto new_position = DirectX::XMVectorAdd(target, dir);
        _camera->SetPosition(new_position);
    }

    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
        auto io = ImGui::GetIO();
        ImVec2 mouse_delta = io.MouseDelta;
        if (io.KeyShift) {
            //Move
            _camera->MoveOnViewSpace(DirectX::XMVectorSet(
                    -mouse_delta.x * DragSpeed, mouse_delta.y * DragSpeed, 0.0f, 0.0f)
            );
        } else {
            //rotation
            _camera_x_angle -= (mouse_delta.x * DragSpeed) * F_PI;
            _camera_y_angle += (mouse_delta.y * DragSpeed) * F_PI;
            if (_camera_x_angle >= 360.0f * F_PI) _camera_x_angle = 0.0f;
            if (_camera_x_angle <= -360.0f * F_PI) _camera_x_angle = 0.0f;
            if (_camera_y_angle >= 360.0f * F_PI) _camera_y_angle = 0.0f;
            if (_camera_y_angle <= -360.0f * F_PI) _camera_y_angle = 0.0f;

            float radius = _camera->GetCameraDistance();
            auto pivot = _camera->GetCameraTarget();
            float new_camera_position_x = (radius * sinf(_camera_x_angle) * cosf(_camera_y_angle)) + pivot.m128_f32[0];
            float new_camera_position_y = (radius * sinf(_camera_y_angle)) + pivot.m128_f32[1];
            float new_camera_position_z =
                    (-(radius * cosf(_camera_x_angle)) * cosf(_camera_y_angle)) + pivot.m128_f32[2];

            auto new_camera_position = DirectX::XMVectorSet(new_camera_position_x, new_camera_position_y,
                                                            new_camera_position_z, 1.0f);

            float new_camera_upvector_x = -sinf(_camera_y_angle) * sinf(_camera_x_angle);
            float new_camera_upvector_y = cosf(_camera_y_angle);
            float new_camera_upvector_z = sinf(_camera_y_angle) * cosf(_camera_x_angle);

            _camera->SetPosition(new_camera_position);
            _camera->LookAt(_camera->GetCameraTarget(),
                            DirectX::XMVectorSet(new_camera_upvector_x, new_camera_upvector_y, new_camera_upvector_z,
                                                 0.0f));
        }
    }
}

void ViewportWindow::UpdateScreen() {
    DirectX::XMStoreFloat4x4(&_screen->ViewMatrix, _camera->GetViewMatrix());
    DirectX::XMStoreFloat4x4(&_screen->ProjectionMatrix, _camera->GetProjectionMatrix());
    DirectX::XMStoreFloat4x4(&_screen->InverseViewMatrix, _camera->GetInverseViewMatrix());
}

std::shared_ptr<_ENGINE::RenderScreen> ViewportWindow::GetRenderScreen() const {
    return _screen;
}
_END_KONAI3D
