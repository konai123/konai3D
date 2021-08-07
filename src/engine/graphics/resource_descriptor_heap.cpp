//
// Created by khhan on 2021-06-16.
//

#include "src/engine/graphics/resource_descriptor_heap.h"
#include "src/engine/graphics/macros.h"

_START_ENGINE
ResourceDescriptorHeap::ResourceDescriptorHeap()
:
_rtv_heap(nullptr),
_dsv_heap(nullptr),
_srv_cbv_uav_heap(nullptr),
_sampler_heap(nullptr),
_render_target_pool(NumRenderTargetDescriptors, false),
_depth_stencil_pool(NumDepthStencilDescriptors, false),
_sampler_pool(NumSamplerDescriptors, false),
_shader_resource_pool(NumShaderResourceDescriptors, false)
{}

bool ResourceDescriptorHeap::Initiate(DeviceCom* device) {
    _rtv_heap = device->CreateRtvHeap(NumRenderTargetDescriptors);
    _dsv_heap = device->CreateDsvHeap(NumDepthStencilDescriptors);
    _srv_cbv_uav_heap = device->CreateSrvCbvUavHeap(NumShaderResourceDescriptors,
                                                    D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    _sampler_heap = device->CreateSamplerHeap(NumSamplerDescriptors, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    if (
            _rtv_heap == nullptr ||
            _dsv_heap == nullptr ||
            _srv_cbv_uav_heap == nullptr ||
            _sampler_heap == nullptr
            ) {
        GRAPHICS_LOG_ERROR("Failed to create DescriptorHeap.");
        return false;
    }

    return true;
}

HeapDescriptorHandle ResourceDescriptorHeap::AllocRenderTargetHeapDescriptor() {
    int idx = _render_target_pool.allocate();
    auto cpu_start = _rtv_heap->GetCPUDescriptorHandleForHeapStart();
    auto gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT());
    auto cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cpu_start, idx, DeviceCom::RtvDescSize);
    return {cpu_handle, gpu_handle, idx};
}

HeapDescriptorHandle ResourceDescriptorHeap::AllocSamplerHeapDescriptor() {
    int idx = _sampler_pool.allocate();
    auto gpu_start = _sampler_heap->GetGPUDescriptorHandleForHeapStart();
    auto cpu_start = _sampler_heap->GetCPUDescriptorHandleForHeapStart();
    auto gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(gpu_start, idx, DeviceCom::SamplerDescSize);
    auto cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cpu_start, idx, DeviceCom::SamplerDescSize);
    return {cpu_handle, gpu_handle, idx};
}

HeapDescriptorHandle ResourceDescriptorHeap::AllocDepthStencilHeapDescriptor() {
    int idx = _depth_stencil_pool.allocate();
    auto cpu_start = _dsv_heap->GetCPUDescriptorHandleForHeapStart();
    auto gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT());
    auto cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cpu_start, idx, DeviceCom::DsvDescSize);
    return {cpu_handle, gpu_handle, idx};
}

HeapDescriptorHandle ResourceDescriptorHeap::AllocShaderResourceHeapDescriptor() {
    int idx = _shader_resource_pool.allocate();
    auto gpu_start = _srv_cbv_uav_heap->GetGPUDescriptorHandleForHeapStart();
    auto cpu_start = _srv_cbv_uav_heap->GetCPUDescriptorHandleForHeapStart();
    auto gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(gpu_start, idx, DeviceCom::SrvCbvUavDescSize);
    auto cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cpu_start, idx, DeviceCom::SrvCbvUavDescSize);
    return {cpu_handle, gpu_handle, idx};
}

void ResourceDescriptorHeap::DiscardRenderTargetHeapDescriptor(int idx) {
    if (idx < 0 || idx >= NumRenderTargetDescriptors) return;
    _render_target_pool.free(idx);
}

void ResourceDescriptorHeap::DiscardSamplerHeapDescriptor(int idx) {
    if (idx < 0 || idx >= NumSamplerDescriptors) return;
    _sampler_pool.free(idx);
}

void ResourceDescriptorHeap::DiscardDepthStencilHeapDescriptor(int idx) {
    if (idx < 0 || idx >= NumDepthStencilDescriptors) return;
    _depth_stencil_pool.free(idx);
}

void ResourceDescriptorHeap::DiscardShaderResourceHeapDescriptor(int idx) {
    if (idx < 0 || idx >= NumShaderResourceDescriptors) return;
    _shader_resource_pool.free(idx);
}
_END_ENGINE