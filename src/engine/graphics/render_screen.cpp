//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/render_screen.h"
#include "src/engine/graphics/renderer.h"

_START_ENGINE
RenderScreen::RenderScreen(
        std::shared_ptr<DeviceCom> device,
        std::shared_ptr<ResourceDescriptorHeap> resource_heap_pool,
        UINT width,
        UINT height,
        DirectX::XMVECTORF32 _clear_color
)
:
_device(std::move(device)),
_resource_heap(std::move(resource_heap_pool)),
_clear_color(_clear_color)
{
    CreateRenderTargets(false, width, height);
};

RenderScreen::~RenderScreen() {
    _resource_heap->DiscardRenderTargetHeapDescriptor(_render_target_view._heap_index);
    _resource_heap->DiscardRenderTargetHeapDescriptor(_shader_view._heap_index);
    _resource_heap->DiscardRenderTargetHeapDescriptor(_depth_stencil_view._heap_index);
}

bool RenderScreen::AddRenderObject(std::string name, RenderObject* renderObject) {
    if (_render_objects.contains(name)) {
        GRAPHICS_LOG_WARNING("RenderObject {} already exist", name);
        return false;
    }
    _render_objects[name] = renderObject;
    return true;
};

bool RenderScreen::UnRegisterRenderObject(std::string name) {
    if (_render_objects.contains(name)) {
        _render_objects.erase(name);
        return true;
    }
    return false;
}

RenderObject* RenderScreen::GetRenderObject(std::string name) {
    if (_render_objects.contains(name)) {
        return _render_objects[name];
    }
    return nullptr;
}

std::vector<std::string> RenderScreen::GetRenderObjectList() {
    std::vector<std::string> ret;
    for (auto &r : _render_objects) {
        ret.push_back(r.first);
    }
    return ret;
}

UINT RenderScreen::Size() {
    return static_cast<UINT>(_render_objects.size());
};

D3D12_VIEWPORT *RenderScreen::GetViewPort() {
    return &_viewport;
};

D3D12_RECT *RenderScreen::GetScissorRect() {
    return &_scissor_rect;
};

ID3D12Resource *RenderScreen::GetRenderTargetResource() {
    return _render_target.Get();
};

ID3D12Resource *RenderScreen::GetDepthStencilResource() {
    return _dsv_buffer.Get();
}

HeapDescriptorHandle *RenderScreen::GetRenderTargetHeapDesc() {
    return &_render_target_view;
};

HeapDescriptorHandle *RenderScreen::GetShaderResourceHeapDesc() {
    return &_shader_view;
};

HeapDescriptorHandle *RenderScreen::GetDepthStencilHeapDesc() {
    return &_depth_stencil_view;
}

bool RenderScreen::Resize(UINT width, UINT height) {
    return CreateRenderTargets(true, width, height);
}

bool RenderScreen::CreateRenderTargets(bool isRecreation, UINT width, UINT height) {
    if (!CreateRenderTargetResourceAndView(isRecreation, width, height)) {
        return false;
    }
    if (!CreateDepthStencilBufferAndView(isRecreation, width, height)) {
        return false;
    }
    _viewport = {0};
    _viewport.Width = static_cast<float>(width);
    _viewport.Height = static_cast<float>(height);
    _viewport.MinDepth = 0.0f;
    _viewport.MaxDepth = 1.0f;
    _scissor_rect = {0, 0, static_cast<int>(width), static_cast<int>(height)};
    Width = width;
    Height = height;
    return true;
}

bool RenderScreen::CreateRenderTargetResourceAndView(bool isRecreation, UINT width, UINT height) {
    CD3DX12_RESOURCE_DESC render_target_desc;
    render_target_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    render_target_desc.Alignment = 0;
    render_target_desc.Width = width;
    render_target_desc.Height = height;
    render_target_desc.DepthOrArraySize = 1;
    render_target_desc.MipLevels = 1;
    render_target_desc.Format = Renderer::BackbufferFormat;
    render_target_desc.SampleDesc.Count = 1;
    render_target_desc.SampleDesc.Quality = 0;
    render_target_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    render_target_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    CD3DX12_CLEAR_VALUE clear_value;
    clear_value.Format = Renderer::BackbufferFormat;
    clear_value.Color[0] = _clear_color.f[0];
    clear_value.Color[1] = _clear_color.f[1];
    clear_value.Color[2] = _clear_color.f[2];
    clear_value.Color[3] = _clear_color.f[3];

    auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    _render_target = nullptr;
    _render_target = _device->CreateResource(
            &properties,
            &render_target_desc,
            &clear_value,
            D3D12_HEAP_FLAG_NONE,
            D3D12_RESOURCE_STATE_COMMON
    );

    if (!isRecreation) {
        _shader_view = _resource_heap->AllocShaderResourceHeapDescriptor();
        _render_target_view = _resource_heap->AllocRenderTargetHeapDescriptor();
    }

    if (!_render_target_view.IsVaild()) {
        GRAPHICS_LOG_ERROR("Failed to create render target view.");
        return false;
    }

    if (!_shader_view.IsVaild()) {
        GRAPHICS_LOG_ERROR("Failed to create shader resource view.");
        return false;
    }

    _device->CreatDescriptorHeapView<D3D12_RENDER_TARGET_VIEW_DESC>(
            _render_target.Get(), nullptr, _render_target_view.CpuHandle, nullptr
    );

    _device->CreatDescriptorHeapView<D3D12_SHADER_RESOURCE_VIEW_DESC>(
            _render_target.Get(), nullptr, _shader_view.CpuHandle, nullptr
    );
    return true;
}

bool RenderScreen::CreateDepthStencilBufferAndView(bool isRecreation, UINT width, UINT height) {
    CD3DX12_RESOURCE_DESC depth_stencil_desc;
    depth_stencil_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depth_stencil_desc.Alignment = 0;
    depth_stencil_desc.Width = width;
    depth_stencil_desc.Height = height;
    depth_stencil_desc.DepthOrArraySize = 1;
    depth_stencil_desc.MipLevels = 1;
    depth_stencil_desc.Format = Renderer::DepthStencilFormat;
    depth_stencil_desc.SampleDesc.Count = 1;
    depth_stencil_desc.SampleDesc.Quality = 0;
    depth_stencil_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depth_stencil_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    CD3DX12_CLEAR_VALUE clear_value;
    clear_value.Format = Renderer::DepthStencilFormat;
    clear_value.DepthStencil.Depth = 1.0f;
    clear_value.DepthStencil.Stencil = 0;

    auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    _dsv_buffer = nullptr;
    _dsv_buffer = _device->CreateResource(
            &properties,
            &depth_stencil_desc,
            &clear_value,
            D3D12_HEAP_FLAG_NONE,
            D3D12_RESOURCE_STATE_DEPTH_WRITE
    );

    if (!isRecreation)
        _depth_stencil_view = _resource_heap->AllocDepthStencilHeapDescriptor();
    if (!_depth_stencil_view.IsVaild()) {
        GRAPHICS_LOG_ERROR("Failed to create depth stencil view.");
        return false;
    }
    _device->CreatDescriptorHeapView<D3D12_DEPTH_STENCIL_VIEW_DESC>(
            _dsv_buffer.Get(), nullptr, _depth_stencil_view.CpuHandle, nullptr
    );
    return true;
}
_END_ENGINE