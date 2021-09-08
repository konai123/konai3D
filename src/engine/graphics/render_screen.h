//
// Created by korona on 2021-06-27.
//

#pragma once

#include <utility>

#include "src/engine/core/pool.hpp"
#include "src/engine/graphics/macros.h"
#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/resource_descriptor_heap.h"
#include "src/engine/graphics/render_object.h"
#include "src/engine/graphics/shader_types.h"


_START_ENGINE

class RenderScreen{
public:
    struct CameraInfo {
        float3 CameraPosition;
        float3 CameraDirection;
        float3 CameraUp;
        float Near;
        float Far;
        float Fov;
        float AspectRatio;
    };

public:
    RenderScreen (
            std::shared_ptr<DeviceCom> device,
            std::shared_ptr<ResourceDescriptorHeap> resource_heap_pool,
            UINT width,
            UINT height,
            DirectX::XMVECTORF32 _clear_color
    );
    virtual ~RenderScreen();

public:
    bool AddRenderObject(std::string name, std::string materialName, std::string meshID, int submeshID);
    bool UnRegisterRenderObject(std::string name);
    RenderObject *GetRenderObject(std::string name);
    std::vector<std::string> GetRenderObjectList();

    bool AddLight(std::string name, ShaderType::LightType lightType);
    bool RemoveLight(std::string name);
    Light *GetLight(std::string name);
    std::vector<std::string> GetLightList();

    D3D12_VIEWPORT *GetViewPort();
    D3D12_RECT *GetScissorRect();
    ID3D12Resource *GetRenderTargetResource();

    HeapDescriptorHandle *GetShaderResourceHeapDesc();

    void SetCameraInfo(const CameraInfo& info);
    CameraInfo GetCameraInfo();

    bool Resize(UINT width, UINT height);
    UINT Size();

public:
    bool Updated;

    UINT Width;
    UINT Height;
    std::optional<std::string> EnvTextureKey;

public:
    inline static const UINT MAX_RENDER_OBJECT = 1000;
    inline static const UINT MAX_LIGHT = 100;

private:
    CameraInfo _camera_info;

private:
    bool CreateRenderTargets(bool isRecreation, UINT width, UINT height);
    bool CreateRenderTargetResourceAndView(bool isRecreation, UINT width, UINT height);

private:
    std::shared_ptr<DeviceCom> _device;
    std::shared_ptr<ResourceDescriptorHeap> _resource_heap;
    std::unordered_map<std::string, RenderObject> _render_objects;
    std::unordered_map<std::string, Light> _lights;

    Microsoft::WRL::ComPtr<ID3D12Resource> _render_target;

    HeapDescriptorHandle _shader_view;
    DirectX::XMVECTORF32 _clear_color;

    D3D12_VIEWPORT _viewport;
    D3D12_RECT _scissor_rect;

};
_END_ENGINE
