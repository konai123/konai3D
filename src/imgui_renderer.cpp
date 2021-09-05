//
// Created by korona on 2021-06-16.
//

#include "src/imgui_renderer.h"
#include "src/global.h"

_START_KONAI3D
IMGUIRenderer::IMGUIRenderer()
:
_app_height(0),
_app_width(0),
_setting_path(""),
_scale_factor(1.0f) {}

bool IMGUIRenderer::OnInitiate(const HWND hwnd,
                               _ENGINE::DeviceCom *device,
                               const DXGI_FORMAT backbufferFormat,
                               ID3D12DescriptorHeap *srvDescriptorHeap,
                               _ENGINE::HeapDescriptorHandle *shaderResourceView,
                               const UINT appWidth,
                               const UINT appHeight,
                               const UINT numFrameFlight
) {

    //Reference: https://github.com/ocornut/imgui/blob/docking/examples/example_win32_directx12/main.cpp
    if (!IMGUI_CHECKVERSION()) {
        CORE_LOG_ERROR("Filed to check imgui version.");
        return false;
    }

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    if (!ImGui_ImplWin32_Init(hwnd)) {
        CORE_LOG_ERROR("Failed to initialize ImplWin32.");
        return false;
    }
    if (!ImGui_ImplDX12_Init(device->GetDevice(), static_cast<int>(numFrameFlight),
                             backbufferFormat, srvDescriptorHeap,
                             shaderResourceView->CpuHandle,
                             shaderResourceView->GpuHandle)) {
        CORE_LOG_ERROR("Failed to initialize ImplDX12.");
        return false;
    }

    {
        /*
         * imgui Settings
         * */
        _setting_path = (global::ExecutablePath / "imgui.ini").string();
        ImGui::LoadIniSettingsFromDisk(_setting_path.data());
    }

    _app_width = appWidth;
    _app_height = appHeight;

    return true;
}

void IMGUIRenderer::OnGUIRender(float delta, ID3D12GraphicsCommandList *cmd_list) {
    if (_window == nullptr) return;
    // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    {
        _window->OnUpdate(delta);
    }

    // Rendering
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmd_list);

    // Update and Render additional Platform Windows
    //ImGui::UpdatePlatformWindows();
    //ImGui::RenderPlatformWindowsDefault(NULL, cmd_list);
}

void IMGUIRenderer::OnResize(UINT width, UINT height) {
    ImGui_ImplDX12_InvalidateDeviceObjects();
    ImGui_ImplDX12_CreateDeviceObjects();
    _app_width = width;
    _app_height = height;
}

void IMGUIRenderer::OnDestroy() {
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void IMGUIRenderer::SetEditor(std::shared_ptr<IMGUIWindow> window) {
    _window = window;
}

void IMGUIRenderer::AddFont(const char *fontPath, const int fontSize) {
    ImGuiIO &io = ImGui::GetIO();
    ImFont *font = io.Fonts->AddFontFromFileTTF(fontPath, static_cast<float>(fontSize));
    if (font == nullptr) {
        CORE_LOG_ERROR("Cannot load fontfile.");
        return;
    }
    io.FontDefault = font;
}

void IMGUIRenderer::SetScale(const float scale) {
    ImGui::GetIO().FontGlobalScale = scale;
    ImGui::GetStyle().ScaleAllSizes(scale / _scale_factor);
    _scale_factor = scale;
}

_END_KONAI3D
