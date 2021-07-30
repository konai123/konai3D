//
// Created by korona on 2021-06-27.
//

#ifndef KONAI3D_RENDER_SCREEN_H
#define KONAI3D_RENDER_SCREEN_H

#include <utility>

#include "src/engine/graphics/macros.h"
#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/resource_descriptor_heap.h"
#include "src/engine/graphics/rendered.h"
#include "src/engine/graphics/render_object.h"

_START_ENGINE
class RenderScreen : public Rendered {
public:
    RenderScreen(
            std::shared_ptr<DeviceCom> device,
            std::shared_ptr<ResourceDescriptorHeap> resource_heap_pool,
            UINT width,
            UINT height,
            DXGI_FORMAT backbuffer_format,
            DXGI_FORMAT depth_stencil_format,
            DirectX::XMVECTORF32 _clear_color
    );
    virtual ~RenderScreen();

public:
    bool AddRenderObject(std::shared_ptr<RenderObject> renderObject);
    bool DeleteRenderObject(std::string name);
    UINT DeletedQueueSize();
    void CollectGarbage();

    std::shared_ptr<RenderObject> GetRenderObject(std::string name);
    std::vector<RenderObject *> GetRenderObjects();

    D3D12_VIEWPORT *GetViewPort();
    D3D12_RECT *GetScissorRect();
    ID3D12Resource *GetRenderTargetResource();
    ID3D12Resource *GetDepthStencilResource();

    HeapDescriptor *GetRenderTargetHeapDesc();
    HeapDescriptor *GetShaderResourceHeapDesc();
    HeapDescriptor *GetDepthStencilHeapDesc();
    DirectX::XMFLOAT3 GetCameraPosition();

    bool Resize(UINT width, UINT height);
    UINT Size();

private:
    bool CreateRenderTargets(bool isRecreation, UINT width, UINT height);
    bool CreateRenderTargetResourceAndView(bool isRecreation, UINT width, UINT height);
    bool CreateDepthStencilBufferAndView(bool isRecreation, UINT width, UINT height);

private:
    std::shared_ptr<DeviceCom> _device;
    std::shared_ptr<ResourceDescriptorHeap> _resource_heap;
    std::list<std::shared_ptr<RenderObject>> _render_objects;
    std::queue<std::shared_ptr<RenderObject>> _deleted_queue;

    Microsoft::WRL::ComPtr<ID3D12Resource> _dsv_buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> _render_target;

    HeapDescriptor _render_target_view;
    HeapDescriptor _shader_view;
    HeapDescriptor _depth_stencil_view;

    DirectX::XMVECTORF32 _clear_color;
    UINT _width;
    UINT _height;
    D3D12_VIEWPORT _viewport;
    D3D12_RECT _scissor_rect;
    DXGI_FORMAT _backbuffer_format;
    DXGI_FORMAT _depth_stencil_format;
};
_END_ENGINE

#endif //KONAI3D_RENDER_SCREEN_H
