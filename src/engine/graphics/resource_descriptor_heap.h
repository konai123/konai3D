//
// Created by khhan on 2021-06-16.
//


#pragma once

#include "src/engine/core/pool.hpp"
#include "src/engine/graphics/device_com.h"

_START_ENGINE
struct HeapDescriptorHandle {
public:
    HeapDescriptorHandle()
    :
            CpuHandle(CD3DX12_DEFAULT()),
            GpuHandle(CD3DX12_DEFAULT()),
            _heap_index(-1) {}

    HeapDescriptorHandle(CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle, int idx)
    :
            CpuHandle(cpuHandle),
            GpuHandle(gpuHandle),
            _heap_index(idx) {}

public:
    bool IsVaild() {
        if (CpuHandle.ptr == NULL)
            return false;
        return true;
    }



public:
    CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE GpuHandle;
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
    virtual bool Initiate(DeviceCom* device);

public:
    virtual HeapDescriptorHandle AllocRenderTargetHeapDescriptor();
    virtual HeapDescriptorHandle AllocSamplerHeapDescriptor();
    virtual HeapDescriptorHandle AllocDepthStencilHeapDescriptor();
    virtual HeapDescriptorHandle AllocShaderResourceHeapDescriptor();

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
};
_END_ENGINE
