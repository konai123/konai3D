//
// Created by khhan on 2021-06-14.
//

#pragma once

#include "src/engine/core/scoped_handle.h"

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/frame_buffer_pool.h"
#include "src/engine/graphics/resource_descriptor_heap.h"
#include "src/engine/graphics/ui_renderer.h"
#include "src/engine/graphics/render_object.h"
#include "src/engine/graphics/render_screen.h"
#include "src/engine/graphics/sampler.h"
#include "src/engine/graphics/macros.h"
#include "src/engine/graphics/mesh_map.h"
#include "src/engine/graphics/texture_map.h"
#include "src/engine/graphics/material_map.h"
#include "src/engine/graphics/resource_garbage_queue.h"
#include "src/engine/graphics/raytracing_pass.h"

_START_ENGINE
struct RenderingOptions {
    bool v_sync;
    float scale_factor;
};

class Renderer {
public:
    Renderer();
    ~Renderer() = default;
    Renderer &operator=(const Renderer &) = delete;
    Renderer &operator=(Renderer &&) noexcept = default;
    Renderer(const Renderer &) = delete;
    Renderer(Renderer &&) noexcept = default;

public:
    bool Initiate(HWND hWnd,
                  UINT width,
                  UINT height,
                  std::filesystem::path shaderDirectoryPath,
                  std::shared_ptr<UIRenderer> uiRenderer = nullptr);
    void OnRender(
            float delta,
            RenderScreen *screen
    );
    void OnResizeGUI(UINT width, UINT height);
    void OnDestroy();
    void SetRenderingOptions(RenderingOptions options);
    void WaitAllFrameExecute();
    UINT GetCurrentFrameIndex() const;
    std::unique_ptr<RenderScreen> InstanceRenderScreen(UINT width, UINT height);

private:
    bool CreateGUIDepthStencilBufferAndView(bool isRecreation, HeapDescriptorHandle *heapDescriptor,
                                            Microsoft::WRL::ComPtr<ID3D12Resource> &resource, UINT width, UINT height);
    bool CreateGUIRenderTargetBufferAndView(bool isRecreation);

public:
    static const UINT NumPreFrames = 3;
    static const size_t NumBackbuffer = 3;
    static const DXGI_FORMAT BackbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    static const DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    inline static DirectX::XMVECTORF32 ClearColor;

public:
    struct ResourceMap {
        std::unique_ptr<MeshMap> MeshMap;
        std::unique_ptr<TextureMap> TextureMap;
        std::unique_ptr<MaterialMap> MaterialMap;
    };
    std::shared_ptr<ResourceMap> RenderResourceMap;

private:
    static DWORD WINAPI UploadWorker(PVOID parameter);
    HANDLE _uplaod_worker_handle;
    std::atomic<bool> _upload_worker_stop;

private:
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> _command_queue;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList6> _command_list;
    Microsoft::WRL::ComPtr<ID3D12Resource> _dsv_buffer_full_frame;

    HeapDescriptorHandle _gui_view;
    HeapDescriptorHandle _full_frame_depth_stencil_view;
    HeapDescriptorHandle _backbuffer_view[NumBackbuffer];

private:
    std::shared_ptr<DeviceCom> _device;
    std::shared_ptr<ResourceDescriptorHeap> _resource_heap;

    UINT _gui_render_target_width;
    UINT _gui_render_target_height;
    D3D12_VIEWPORT _gui_viewport;
    D3D12_RECT _gui_scissor_rect;
    FrameBufferPool _frame_buffer_pool;
    RenderingOptions _rendering_options;
    std::unique_ptr<ScopedHandle> _worker_event;

    std::shared_ptr<UIRenderer> _ui_renderer;
    UINT _current_frame;

private:
    /*Render Passes*/
    std::unique_ptr<Raytracer> _render_pass;
};

_END_ENGINE
