//
// Created by korona on 2021-06-10.
//

#pragma once

#include "src/engine/graphics/dx_com.h"

_START_ENGINE
class DeviceCom {
public:
    DeviceCom();
    virtual ~DeviceCom() = default;
    DeviceCom &operator=(const DeviceCom &) = delete;
    DeviceCom &operator=(DeviceCom &&) noexcept = default;
    DeviceCom(const DeviceCom &) = delete;
    DeviceCom(DeviceCom &&) noexcept = default;

public:
    bool Initiate(const HWND hWnd, const UINT width, const UINT height, const size_t swapchainCount,
                  const DXGI_FORMAT backbufferFormat);

public:
    Microsoft::WRL::ComPtr<IDXGISwapChain3> CreateSwapchain(
            const HWND hWnd, const UINT width, const UINT height, const size_t swapchainCount,
            ID3D12CommandQueue *commandQueue,
            const DXGI_FORMAT backbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM
    );
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> CreateCommandQueue(
            const D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            const D3D12_COMMAND_QUEUE_FLAGS flag = D3D12_COMMAND_QUEUE_FLAG_NONE,
            const D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL
    );
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(
            const D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT
    );
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CreateGraphicsCommandList(
            const D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            const D3D12_COMMAND_LIST_FLAGS flags = D3D12_COMMAND_LIST_FLAG_NONE
    );
    Microsoft::WRL::ComPtr<ID3D12Fence> CreateFence(unsigned __int64 initialValue);
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateRtvHeap(
            const int descCount,
            const D3D12_DESCRIPTOR_HEAP_FLAGS flag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
    );
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDsvHeap(
            const int descCount,
            const D3D12_DESCRIPTOR_HEAP_FLAGS flag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
    );
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateSrvCbvUavHeap(
            const int descCount,
            const D3D12_DESCRIPTOR_HEAP_FLAGS flag = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
    );
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateSamplerHeap(
            const int descCount,
            const D3D12_DESCRIPTOR_HEAP_FLAGS flag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
    );
    Microsoft::WRL::ComPtr<ID3D12PipelineState> CreatePipelineState(
            const D3D12_GRAPHICS_PIPELINE_STATE_DESC *desc
    );
    Microsoft::WRL::ComPtr<ID3D12RootSignature> CreateRootSignature(
            const CD3DX12_ROOT_PARAMETER1 *parameters,
            const CD3DX12_STATIC_SAMPLER_DESC *staticSamplers,
            const UINT numStaticSampler,
            const UINT numParameter,
            const D3D12_ROOT_SIGNATURE_FLAGS flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateResource(
            CD3DX12_HEAP_PROPERTIES *heapProperties,
            CD3DX12_RESOURCE_DESC *resourceDesc,
            CD3DX12_CLEAR_VALUE *clearValue,
            const D3D12_HEAP_FLAGS heapFlags,
            const D3D12_RESOURCE_STATES resourceState
    );
    bool CreateSamplerResource(D3D12_SAMPLER_DESC *desc, D3D12_CPU_DESCRIPTOR_HANDLE handle);
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue();
    Microsoft::WRL::ComPtr<ID3D12Resource> GetCurrentBackBuffer();
    Microsoft::WRL::ComPtr<ID3D12Resource> GetBackBuffer(int idx);
    UINT GetCurrentBackBufferIndex();
public:
    template<typename TPipelineState>
    Microsoft::WRL::ComPtr<ID3D12PipelineState> CreatePipelineStateStream(
            const TPipelineState &psoDesc
    );
    template<typename tHeapDesc>
    constexpr void CreatDescriptorHeapView(
            ID3D12Resource *resource,
            ID3D12Resource *counterResource,
            const CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle,
            tHeapDesc *heapDesc
    );

public:
    ID3D12Device5 *GetDevice() const;
    bool Present(bool VSync);
    bool ResizeSwapchain(const UINT width, const UINT height);
    void Close();

private:
    bool LoadDxCom();

public:
    inline static UINT RtvDescSize = -1;
    inline static UINT SrvCbvUavDescSize = -1;
    inline static UINT DsvDescSize = -1;
    inline static UINT SamplerDescSize = -1;

private:
    std::unique_ptr<DXCom> _dx_com;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> _command_queue;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> _swapchain;
};
_END_ENGINE

#include "device_com.tcc"
