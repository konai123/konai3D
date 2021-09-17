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
_clear_color(_clear_color),
EnvTextureKey(std::nullopt),
Updated(true)
{
    CreateRenderTargets(false, width, height);
};

RenderScreen::~RenderScreen() {
    _resource_heap->DiscardRenderTargetHeapDescriptor(_shader_view._heap_index);
}

bool RenderScreen::AddRenderObject(std::string name, std::string materialName, std::string meshID, int submeshID) {
    if (_render_objects.contains(name)) {
        GRAPHICS_LOG_WARNING("RenderObject {} already exist", name);
        return false;
    }

    auto newRenderObj = _ENGINE::RenderObject::AllocRenderObject();
    if (!newRenderObj.has_value()) return false;

    newRenderObj->MaterialName = materialName;
    newRenderObj->MeshID = meshID;
    newRenderObj->SubmeshID = submeshID;

    _render_objects[name] = newRenderObj.value();
    Updated = true;
    return true;
};

bool RenderScreen::UnRegisterRenderObject(std::string name) {
    if (_render_objects.contains(name)) {
        RenderObject::DiscardRenderObject(_render_objects[name]);
        _render_objects.erase(name);
        Updated = true;
        return true;
    }
    return false;
}

RenderObject* RenderScreen::GetRenderObject(std::string name) {
    if (_render_objects.contains(name)) {
        return &_render_objects[name];
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

bool RenderScreen::AddLight(std::string name, ShaderType::LightType lightType) {
    if (_lights.contains(name)) {
        GRAPHICS_LOG_WARNING("Light {} already exist", name);
        return false;
    }
    Light light;
    light.LightType = lightType;
    light.Position = {0.0f, 0.0f, 0.0f};
    light.I = 1.0f;
    _lights[name] = light;
    Updated = true;
    return true;
}

bool RenderScreen::RemoveLight(std::string name) {
    if (!_lights.contains(name)) {
        GRAPHICS_LOG_WARNING("Light {} doesn't exist", name);
        return false;
    }
    _lights.erase(name);
    Updated = true;
    return true;
}

Light *RenderScreen::GetLight(std::string name) {
    if (!_lights.contains(name)) {
        GRAPHICS_LOG_WARNING("Light {} doesn't exist", name);
        return nullptr;
    }
    return &_lights[name];
}

std::vector<std::string> RenderScreen::GetLightList() {
    std::vector<std::string> lights;
    for (auto& p : _lights) {
        lights.push_back(p.first);
    }
    return lights;
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

HeapDescriptorHandle *RenderScreen::GetShaderResourceHeapDesc() {
    return &_shader_view;
};

bool RenderScreen::Resize(UINT width, UINT height) {
    Updated = true;
    return CreateRenderTargets(true, width, height);
}

bool RenderScreen::CreateRenderTargets(bool isRecreation, UINT width, UINT height) {
    if (!CreateRenderTargetResourceAndView(isRecreation, width, height)) {
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
    render_target_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    if (_render_target != nullptr) {
        ResourceGarbageQueue::Instance().SubmitResource(_render_target);
    }
    _render_target = _device->CreateResource(
            &properties,
            &render_target_desc,
            NULL,
            D3D12_HEAP_FLAG_NONE,
            D3D12_RESOURCE_STATE_COMMON
    );

    if (!isRecreation) {
        _shader_view = _resource_heap->AllocShaderResourceHeapDescriptor();
    }

    if (!_shader_view.IsVaild()) {
        GRAPHICS_LOG_ERROR("Failed to create shader resource view.");
        return false;
    }

    _device->CreatDescriptorHeapView<D3D12_SHADER_RESOURCE_VIEW_DESC>(
            _render_target.Get(), nullptr, _shader_view.CpuHandle, nullptr
    );
    return true;
}


RenderScreen::CameraInfo RenderScreen::GetCameraInfo() {
    return _camera_info;
}

void RenderScreen::SetCameraInfo(const CameraInfo& info) {
    if (::memcmp(&_camera_info, &info, sizeof(CameraInfo)) != 0) {
        Updated = true;
        _camera_info = info;
    }
}


_END_ENGINE