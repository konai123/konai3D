//
// Created by khhan on 2021-06-16.
//

#ifndef KONAI3D_RESOURCE_DESCRIPTOR_HEAP_H
#define KONAI3D_RESOURCE_DESCRIPTOR_HEAP_H

#include "src/engine/core/pool.hpp"
#include "src/engine/graphics/device_com.h"

_START_ENGINE
struct HeapDescriptor {
public:
    HeapDescriptor()
    :
    _cpu_handle(CD3DX12_DEFAULT()),
    _gpu_handle(CD3DX12_DEFAULT()),
    _heap_index(-1) {}

    HeapDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle, int idx)
    :
    _cpu_handle(cpuHandle),
    _gpu_handle(gpuHandle),
    _heap_index(idx) {}

public:
    bool IsVaild() {
        if (_cpu_handle.ptr == NULL)
            return false;
        return true;
    }

public:
    CD3DX12_CPU_DESCRIPTOR_HANDLE _cpu_handle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE _gpu_handle;
    int _heap_index;
};

class ResourceDescriptorHeap {
public:
    enum {
        NumRenderTargetDescriptors = 512,
        NumDepthStencilDescriptors = 512,
        NumSamplerDescriptors = 512,
        /*SRV CBV UAV Descriptors*/
        NumShaderResourceDescriptors = 512 * 2
    };

public:
    ResourceDescriptorHeap();
    virtual ~ResourceDescriptorHeap() = default;

public:
    virtual bool Initiate(DeviceCom *device);

public:
    virtual HeapDescriptor GetRenderTargetHeapDescriptor();
    virtual HeapDescriptor GetSamplerHeapDescriptor();
    virtual HeapDescriptor GetDepthStencilHeapDescriptor();
    virtual HeapDescriptor GetShaderResourceHeapDescriptor();
    virtual void DiscardRenderTargetHeapDescriptor(int idx);
    virtual void DiscardSamplerHeapDescriptor(int idx);
    virtual void DiscardDepthStencilHeapDescriptor(int idx);
    virtual void DiscardShaderResourceHeapDescriptor(int idx);
    virtual Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetRenderTargetHeap() const { return _rtv_heap; };
    virtual Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDepthStencilHeap() const { return _dsv_heap; };
    virtual Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetShaderResourceHeap() const { return _srv_cbv_uav_heap; };
    virtual Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSamplerHeap() const { return _sampler_heap; };

private:
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _rtv_heap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _dsv_heap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _srv_cbv_uav_heap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _sampler_heap;

private:
    Pool<void> _render_target_pool;
    Pool<void> _depth_stencil_pool;
    Pool<void> _sampler_pool;
    Pool<void> _shader_resource_pool;

private:
    UINT _rtv_inc_size;
    UINT _dsv_inc_size;
    UINT _sampler_inc_size;
    UINT _srv_cbv_uav_inc_size;
};
_END_ENGINE

#endif //KONAI3D_RESOURCE_DESCRIPTOR_HEAP_H
