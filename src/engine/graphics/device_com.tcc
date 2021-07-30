//
// Created by korona on 2021-06-14.
//

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/macros.h"
#include "src/engine/core/macros.h"

_START_ENGINE
template<typename TPipelineState>
Microsoft::WRL::ComPtr<ID3D12PipelineState> DeviceCom::CreatePipelineStateStream(
        const TPipelineState &psoDesc
) {
    EngineAssert(_dx_com.get());
    ID3D12Device2 *device = _dx_com->Device();
    if (device == nullptr) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
    D3D12_PIPELINE_STATE_STREAM_DESC pipeline_state_desc = {sizeof(TPipelineState), &psoDesc};
    ReturnNullHRFailed(device->CreatePipelineState(
            &pipeline_state_desc, IID_PPV_ARGS(pso.GetAddressOf()))
    );
    return pso;
}

template<typename tHeapDesc>
constexpr void DeviceCom::CreatDescriptorHeapView(
        ID3D12Resource *resource,
        ID3D12Resource *counterResource,
        const CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle,
        tHeapDesc *heapDesc
) {
    EngineAssert(_dx_com.get());
    ID3D12Device *device = _dx_com->Device();
    if (device == nullptr) {
        GRAPHICS_LOG_ERROR("Cannot create a descriptor heap view. Device was null.");
        return;
    }

    if constexpr(std::is_same<tHeapDesc, D3D12_RENDER_TARGET_VIEW_DESC>::value) {
        device->CreateRenderTargetView(resource, heapDesc, heapHandle);
    }
    if constexpr (std::is_same<tHeapDesc, D3D12_DEPTH_STENCIL_VIEW_DESC>::value) {
        device->CreateDepthStencilView(resource, heapDesc, heapHandle);
    }
    if constexpr (std::is_same<tHeapDesc, D3D12_SHADER_RESOURCE_VIEW_DESC>::value) {
        device->CreateShaderResourceView(resource, heapDesc, heapHandle);
    }
    if constexpr(std::is_same<tHeapDesc, D3D12_UNORDERED_ACCESS_VIEW_DESC>::value) {
        device->CreateUnorderedAccessView(resource, counterResource, heapDesc, heapHandle);
    }
    if constexpr(std::is_same<tHeapDesc, D3D12_CONSTANT_BUFFER_VIEW_DESC>::value) {
        device->CreateConstantBufferView(heapDesc, heapHandle);
    }
    if constexpr(std::is_same<tHeapDesc, D3D12_SAMPLER_DESC>::value) {
        device->CreateSampler(heapDesc, heapHandle);
    }
}
_END_ENGINE