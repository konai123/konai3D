//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/render_screen.h"

_START_ENGINE
RenderScreen::RenderScreen(
        std::shared_ptr<DeviceCom> device,
        std::shared_ptr<ResourceDescriptorHeap> resource_heap_pool,
        UINT width,
        UINT height,
        DXGI_FORMAT backbuffer_format,
        DXGI_FORMAT depth_stencil_format,
        DirectX::XMVECTORF32 _clear_color
)
:
_device(std::move(device)),
_resource_heap(std::move(resource_heap_pool)),
_backbuffer_format(backbuffer_format),
_depth_stencil_format(depth_stencil_format),
_clear_color(_clear_color) {
    CreateRenderTargets(false, width, height);
};

RenderScreen::~RenderScreen() {
    _resource_heap->DiscardRenderTargetHeapDescriptor(_render_target_view._heap_index);
    _resource_heap->DiscardRenderTargetHeapDescriptor(_shader_view._heap_index);
    _resource_heap->DiscardRenderTargetHeapDescriptor(_depth_stencil_view._heap_index);
}

bool RenderScreen::AddRenderObject(std::shared_ptr<RenderObject> renderObject) {
    _render_objects.push_back(std::move(renderObject));
    return true;
};

//Delete at front.
bool RenderScreen::DeleteRenderObject(std::string name) {
    for (auto i = _render_objects.begin(); i != _render_objects.end(); i++) {
        auto &r = *i;
        if (r->GetName() == name) {
            _deleted_queue.push(r);
            _render_objects.erase(i);
            return true;
        }
    }
    return false;
}

UINT RenderScreen::DeletedQueueSize() {
    return static_cast<UINT>(_deleted_queue.size());
}

void RenderScreen::CollectGarbage() {
    if (_deleted_queue.size() <= 0) return;
    while (!_deleted_queue.empty()) {
        _deleted_queue.pop();
    }
}

std::shared_ptr<RenderObject> RenderScreen::GetRenderObject(std::string name) {
    for (auto i = _render_objects.begin(); i != _render_objects.end(); i++) {
        auto &r = *i;
        if (r->GetName() == name) {
            return *i;
        }
    }
    return nullptr;
}

std::vector<RenderObject *> RenderScreen::GetRenderObjects() {
    std::vector<RenderObject *> ret;
    for (auto &r : _render_objects) {
        ret.push_back(r.get());
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

HeapDescriptor *RenderScreen::GetRenderTargetHeapDesc() {
    return &_render_target_view;
};

HeapDescriptor *RenderScreen::GetShaderResourceHeapDesc() {
    return &_shader_view;
};

HeapDescriptor *RenderScreen::GetDepthStencilHeapDesc() {
    return &_depth_stencil_view;
}

DirectX::XMFLOAT3 RenderScreen::GetCameraPosition() {
    return {0.0f, 0.0f, 0.0f};
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
    _width = width;
    _height = height;
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
    render_target_desc.Format = _backbuffer_format;
    render_target_desc.SampleDesc.Count = 1;
    render_target_desc.SampleDesc.Quality = 0;
    render_target_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    render_target_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    CD3DX12_CLEAR_VALUE clear_value;
    clear_value.Format = _backbuffer_format;
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
        _shader_view = _resource_heap->GetShaderResourceHeapDescriptor();
        _render_target_view = _resource_heap->GetRenderTargetHeapDescriptor();
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
            _render_target.Get(), nullptr, _render_target_view._cpu_handle, nullptr
    );

    _device->CreatDescriptorHeapView<D3D12_SHADER_RESOURCE_VIEW_DESC>(
            _render_target.Get(), nullptr, _shader_view._cpu_handle, nullptr
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
    depth_stencil_desc.Format = _depth_stencil_format;
    depth_stencil_desc.SampleDesc.Count = 1;
    depth_stencil_desc.SampleDesc.Quality = 0;
    depth_stencil_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depth_stencil_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    CD3DX12_CLEAR_VALUE clear_value;
    clear_value.Format = _depth_stencil_format;
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
        _depth_stencil_view = _resource_heap->GetDepthStencilHeapDescriptor();
    if (!_depth_stencil_view.IsVaild()) {
        GRAPHICS_LOG_ERROR("Failed to create depth stencil view.");
        return false;
    }
    _device->CreatDescriptorHeapView<D3D12_DEPTH_STENCIL_VIEW_DESC>(
            _dsv_buffer.Get(), nullptr, _depth_stencil_view._cpu_handle, nullptr
    );
    return true;
}
_END_ENGINE