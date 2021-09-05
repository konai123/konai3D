//
// Created by korona on 2021-06-16.
//

#pragma once

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/resource_descriptor_heap.h"
#include "src/engine/graphics/ui_renderer.h"

#include "src/editor/imgui_window.h"

_START_KONAI3D
class IMGUIRenderer : public _ENGINE::UIRenderer {
public:
    IMGUIRenderer();
    virtual ~IMGUIRenderer() = default;

public:
    virtual bool OnInitiate(
            const HWND hwnd,
            _ENGINE::DeviceCom *device,
            const DXGI_FORMAT backbufferFormat,
            ID3D12DescriptorHeap *srvDescriptorHeap,
            _ENGINE::HeapDescriptorHandle *shaderResourceView,
            const UINT appWidth,
            const UINT appHeight,
            const UINT numFrameFlight = 3u
    );
    virtual void OnGUIRender(float delta, ID3D12GraphicsCommandList *cmd_list);
    virtual void OnResize(UINT width, UINT height);
    virtual void OnDestroy();

public:
    virtual void SetEditor(std::shared_ptr<IMGUIWindow> window);
    virtual void SetScale(const float scale);
    virtual void AddFont(const char *fontPath, const int fontSize = 13);

private:
    UINT _app_width;
    UINT _app_height;
    std::shared_ptr<IMGUIWindow> _window;
    std::string _setting_path;
    float _scale_factor = 1.0f;
};
_END_KONAI3D
