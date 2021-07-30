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
_shader_resource_pool(NumShaderResourceDescriptors, false),
_rtv_inc_size(0),
_dsv_inc_size(0),
_sampler_inc_size(0),
_srv_cbv_uav_inc_size(0) {}

bool ResourceDescriptorHeap::Initiate(DeviceCom *device) {
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

    _rtv_inc_size = device->_rtv_desc_size;
    _dsv_inc_size = device->_dsv_desc_size;
    _sampler_inc_size = device->_sampler_desc_size;
    _srv_cbv_uav_inc_size = device->_srv_cbv_uav_desc_size;

    return true;
}

HeapDescriptor ResourceDescriptorHeap::GetRenderTargetHeapDescriptor() {
    int idx = _render_target_pool.allocate();
    auto cpu_start = _rtv_heap->GetCPUDescriptorHandleForHeapStart();
    auto gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT());
    auto cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cpu_start, idx, _rtv_inc_size);
    return {cpu_handle, gpu_handle, idx};
}

HeapDescriptor ResourceDescriptorHeap::GetSamplerHeapDescriptor() {
    int idx = _sampler_pool.allocate();
    auto gpu_start = _sampler_heap->GetGPUDescriptorHandleForHeapStart();
    auto cpu_start = _sampler_heap->GetCPUDescriptorHandleForHeapStart();
    auto gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(gpu_start, idx, _sampler_inc_size);
    auto cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cpu_start, idx, _sampler_inc_size);
    return {cpu_handle, gpu_handle, idx};
}

HeapDescriptor ResourceDescriptorHeap::GetDepthStencilHeapDescriptor() {
    int idx = _depth_stencil_pool.allocate();
    auto cpu_start = _dsv_heap->GetCPUDescriptorHandleForHeapStart();
    auto gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT());
    auto cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cpu_start, idx, _dsv_inc_size);
    return {cpu_handle, gpu_handle, idx};
}

HeapDescriptor ResourceDescriptorHeap::GetShaderResourceHeapDescriptor() {
    int idx = _shader_resource_pool.allocate();
    auto gpu_start = _srv_cbv_uav_heap->GetGPUDescriptorHandleForHeapStart();
    auto cpu_start = _srv_cbv_uav_heap->GetCPUDescriptorHandleForHeapStart();
    auto gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(gpu_start, idx, _srv_cbv_uav_inc_size);
    auto cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(cpu_start, idx, _srv_cbv_uav_inc_size);
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