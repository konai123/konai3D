//
// Created by khhan on 2021-06-14.
//

#include "src/engine/graphics/renderer.h"
#include "src/engine/graphics/render_screen.h"
#include "src/engine/graphics/shader.h"

_START_ENGINE
Renderer::Renderer()
:
_command_queue(nullptr),
_command_list(nullptr),
_dsv_buffer_full_frame(nullptr),
_gui_render_target_width(0),
_gui_render_target_height(0),
_gui_viewport({0}),
_gui_scissor_rect({0}),
_current_frame(0),
RenderResourceMap(nullptr),
_upload_worker_stop(false)
{
    _rendering_options = {.v_sync=true, .scale_factor = 1.0f};
}

bool
Renderer::Initiate(HWND hWnd, UINT width, UINT height, std::filesystem::path shaderDirectoryPath,
                   std::shared_ptr<UIRenderer> uiRenderer) {
    _gui_render_target_width = width;
    _gui_render_target_height = height;

    _device = std::make_shared<DeviceCom>();
    _resource_heap = std::make_shared<ResourceDescriptorHeap>();

    if (!_device->Initiate(hWnd, width, height, NumBackbuffer, BackbufferFormat)) {
        GRAPHICS_LOG_ERROR("Failed to initialize direct12.");
        return false;
    }

    {
        /*Compile Shaders*/
        if (!Shader::RenderPass.Build(shaderDirectoryPath)) {
            GRAPHICS_LOG_ERROR("Filed to build shaders");
            return false;
        }
    }

    _command_queue = _device->GetCommandQueue();
    if (_command_queue == nullptr) {
        GRAPHICS_LOG_ERROR("Failed to create a CommandQueue.");
        return false;
    }

    _command_list = _device->CreateGraphicsCommandList();
    if (_command_list == nullptr) {
        GRAPHICS_LOG_ERROR("Failed to create a CommandList.");
        return false;
    }

    if (!_frame_buffer_pool.Initiate(_device.get(), _command_queue, NumPreFrames, D3D12_COMMAND_LIST_TYPE_DIRECT)) {
        GRAPHICS_LOG_ERROR("Failed to initialize CommandExecutor.");
        return false;
    };

    if (!_resource_heap->Initiate(_device.get())) {
        GRAPHICS_LOG_ERROR("Failed to initialize ResourceDescriptorHeap.");
        return false;
    }

    if (!CreateGUIRenderTargetBufferAndView(false))
        return false;

    if (!CreateGUIDepthStencilBufferAndView(false, &_full_frame_depth_stencil_view, _dsv_buffer_full_frame,
                                            _gui_render_target_width, _gui_render_target_height))
        return false;

    _gui_viewport = {0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
    _gui_scissor_rect = {0, 0, static_cast<int>(width), static_cast<int>(height)};

    _gui_view = _resource_heap->AllocShaderResourceHeapDescriptor();

    if (uiRenderer != nullptr) {
        _ui_renderer = uiRenderer;
        if (!_ui_renderer->Initiate(
                hWnd, _device.get(), BackbufferFormat, _resource_heap->GetShaderResourceHeap().Get(), &_gui_view,
                width, height, NumPreFrames)
                ) {
            GRAPHICS_LOG_ERROR("Failed to initialize imgui renderer.");
            return false;
        }
    }

    {
        /*Initiate Render Pass*/
        _render_pass = std::make_unique<RenderPass>(_device);
        if (!_render_pass->Initiate()) {
            GRAPHICS_LOG_ERROR("Failed to initialize render pass");
            return false;
        }
    }

    {
        /*Initialte Render Resource Maps*/
        RenderResourceMap = std::make_shared<ResourceMap>();
        RenderResourceMap->MeshMap = std::make_unique<MeshMap>(_device);
        RenderResourceMap->TextureMap = std::make_unique<TextureMap>(_device, _resource_heap);
        RenderResourceMap->MaterialMap = std::make_unique<MaterialMap>();
    }

    _uplaod_worker_handle = ::CreateThread(NULL, 0, UploadWorker, this, 0, NULL);

    return true;
}

void Renderer::OnRender(
        float delta,
        std::vector<RenderScreen *> screens
) {
    auto frame_buffer = _frame_buffer_pool.RequestFrameBuffer(_device.get());
    frame_buffer->_allocator->Reset();
    _command_list->Reset(frame_buffer->_allocator.Get(), NULL);

    ID3D12DescriptorHeap *heaps[] = {
            _resource_heap->GetShaderResourceHeap().Get(),
            _resource_heap->GetSamplerHeap().Get()
    };

    _command_list->SetDescriptorHeaps(_countof(heaps), heaps);

    {
        //RenderPass.Render();
        _render_pass->Render(delta, screens, _command_list.Get(), _current_frame, RenderResourceMap->MeshMap.get(),
                             RenderResourceMap->TextureMap.get(), RenderResourceMap->MaterialMap.get(), _resource_heap.get());
    }

    _command_list->RSSetViewports(1, &_gui_viewport);
    _command_list->RSSetScissorRects(1, &_gui_scissor_rect);

    D3D12_RESOURCE_BARRIER barrier_enter = CD3DX12_RESOURCE_BARRIER::Transition(
            _device->GetCurrentBackBuffer().Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    _command_list->ResourceBarrier(1, &barrier_enter);

    UINT backbuffer_idx = _device->GetCurrentBackBufferIndex();
    _command_list->OMSetRenderTargets(1, &_backbuffer_view[backbuffer_idx].CpuHandle, true,
                                      &_full_frame_depth_stencil_view.CpuHandle);

    _command_list->ClearRenderTargetView(_backbuffer_view[backbuffer_idx].CpuHandle, ClearColor, 0, nullptr);
    _command_list->ClearDepthStencilView(_full_frame_depth_stencil_view.CpuHandle,
                                         D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                                         1.0f, 0, 0, nullptr);


    /*
     * IMGUI Render pass
     * */
    {
        _ui_renderer->OnGUIRender(delta, _command_list.Get());
    }

    D3D12_RESOURCE_BARRIER barrier_out = CD3DX12_RESOURCE_BARRIER::Transition(
            _device->GetCurrentBackBuffer().Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT
    );
    _command_list->ResourceBarrier(1, &barrier_out);

    _command_list->Close();

    ID3D12CommandList *cmd_list[] = {_command_list.Get()};
    _command_queue->ExecuteCommandLists(_countof(cmd_list), cmd_list);
    _device->Present(_rendering_options.v_sync);
    _frame_buffer_pool.DiscardFrameBuffer(frame_buffer);
    _current_frame = (_current_frame + 1) % (NumPreFrames);
}

void Renderer::OnResizeGUI(UINT width, UINT height) {
    _gui_render_target_width = width;
    _gui_render_target_height = height;
    _gui_viewport.Width = static_cast<float>(width);
    _gui_viewport.Height = static_cast<float>(height);
    _gui_scissor_rect = {0, 0, static_cast<int>(width), static_cast<int>(height)};

    WaitAllFrameExecute();
    _device->ResizeSwapchain(_gui_render_target_width, _gui_render_target_height);
    CreateGUIDepthStencilBufferAndView(true, &_full_frame_depth_stencil_view, _dsv_buffer_full_frame,
                                       _gui_render_target_width,
                                       _gui_render_target_height);
    CreateGUIRenderTargetBufferAndView(true);

    _ui_renderer->OnResize(width, height);
}

void Renderer::OnDestroy() {
    WaitAllFrameExecute();
    _ui_renderer->OnDestroy();
    _device->Close();

    _upload_worker_stop.store(true); //Exit Upload thread
    ::WaitForSingleObject(_uplaod_worker_handle, INFINITE);
}

bool Renderer::CreateGUIDepthStencilBufferAndView(bool isRecreation, HeapDescriptorHandle *heapDescriptor,
                                                  Microsoft::WRL::ComPtr<ID3D12Resource> &resource, UINT width,
                                                  UINT height) {
//Create Depth Stencil Buffer
    CD3DX12_RESOURCE_DESC depth_stencil_desc;
    depth_stencil_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depth_stencil_desc.Alignment = 0;
    depth_stencil_desc.Width = width;
    depth_stencil_desc.Height = height;
    depth_stencil_desc.DepthOrArraySize = 1;
    depth_stencil_desc.MipLevels = 1;
    depth_stencil_desc.Format = DepthStencilFormat;
    depth_stencil_desc.SampleDesc.Count = 1;
    depth_stencil_desc.SampleDesc.Quality = 0;
    depth_stencil_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depth_stencil_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    CD3DX12_CLEAR_VALUE clear_value;
    clear_value.Format = DepthStencilFormat;
    clear_value.DepthStencil.Depth = 1.0f;
    clear_value.DepthStencil.Stencil = 0;

    auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    resource = nullptr;
    resource = _device->CreateResource(
            &properties,
            &depth_stencil_desc,
            &clear_value,
            D3D12_HEAP_FLAG_NONE,
            D3D12_RESOURCE_STATE_DEPTH_WRITE
    );

    if (!isRecreation)
        *heapDescriptor = _resource_heap->AllocDepthStencilHeapDescriptor();
    if (!heapDescriptor->IsVaild()) {
        CORE_LOG_ERROR("Failed to create depth stencil view.");
        return false;
    }
    _device->CreatDescriptorHeapView<D3D12_DEPTH_STENCIL_VIEW_DESC>(
            resource.Get(), nullptr, heapDescriptor->CpuHandle, nullptr
    );

    return true;
}

bool Renderer::CreateGUIRenderTargetBufferAndView(bool isRecreation) {
    //Save Backbuffer from swapchains
    for (int i = 0; i < NumBackbuffer; i++) {
        auto backbuffer = _device->GetBackBuffer(i);

        if (!isRecreation) {
            auto heap_desc = _resource_heap->AllocRenderTargetHeapDescriptor();
            _backbuffer_view[i] = heap_desc;
        }

        if (!_backbuffer_view[i].IsVaild()) {
            CORE_LOG_ERROR("Failed to create rendertarget view.");
            return false;
        }
        _device->CreatDescriptorHeapView<D3D12_RENDER_TARGET_VIEW_DESC>
                (backbuffer.Get(), nullptr, _backbuffer_view[i].CpuHandle, nullptr);
    }

    return true;
}

void Renderer::SetRenderingOptions(RenderingOptions options) {
    _rendering_options = options;
}


void Renderer::WaitAllFrameExecute() {
    _frame_buffer_pool.WaitAllExecute();
}

UINT Renderer::GetCurrentFrameIndex() const {
    return _current_frame;
}

std::unique_ptr<RenderScreen> Renderer::InstanceRenderScreen(UINT width, UINT height) {
    auto render_screen = std::move(std::make_unique<RenderScreen>(_device, _resource_heap, width, height, ClearColor));
    return std::move(render_screen);
}

DWORD Renderer::UploadWorker(PVOID context) {
    Renderer* pthis  = reinterpret_cast<Renderer*>(context);
    DirectX::ResourceUploadBatch uploader(pthis->_device->GetDevice());
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> copy_queue = pthis->_device->CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);

    GRAPHICS_LOG_INFO("Begin Uploader Thread");
    while (!pthis->_upload_worker_stop.load()) {
        uploader.Begin(D3D12_COMMAND_LIST_TYPE_COPY);

        pthis->RenderResourceMap->MeshMap->UpdateFromMeshLoader(&uploader);
        pthis->RenderResourceMap->TextureMap->UpdateFromTextureLoader(&uploader);

        auto future = uploader.End(copy_queue.Get());
        future.wait();
    }
    GRAPHICS_LOG_INFO("End Uploader Thread");
    return 0;
}

_END_ENGINE
