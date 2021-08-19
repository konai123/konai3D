//
// Created by korona on 2021-06-10.
//

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/macros.h"

_START_ENGINE
DeviceCom::DeviceCom()
:
_dx_com(nullptr),
_swapchain(nullptr),
_command_queue(nullptr)
{}

bool DeviceCom::Initiate(const HWND hWnd, const UINT width, const UINT height, const size_t swapchainCount,
                         const DXGI_FORMAT backbufferFormat) {
    if (!LoadDxCom()) {
        GRAPHICS_LOG_ERROR("Failed to load DXGI Interface.");
        return false;
    }
    _command_queue = CreateCommandQueue();
    if (_command_queue == nullptr) {
        GRAPHICS_LOG_ERROR("Failed to load command queue.");
        return false;
    }
    _swapchain.Reset();
    _swapchain = CreateSwapchain(hWnd, width, height, swapchainCount, _command_queue.Get(), backbufferFormat);
    if (_swapchain == nullptr) {
        GRAPHICS_LOG_ERROR("Failed to load swapchain.");
        return false;
    }

    ID3D12Device *device = _dx_com->Device();
    RtvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    DsvDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    SrvCbvUavDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    SamplerDescSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    return true;
}

Microsoft::WRL::ComPtr<IDXGISwapChain3>
DeviceCom::CreateSwapchain(
        const HWND hWnd,
        const UINT width,
        const UINT height,
        const size_t swapchainCount,
        ID3D12CommandQueue *commandQueue,
        const DXGI_FORMAT backbufferFormat) {
    EngineAssert(_dx_com.get());
    IDXGIFactory4 *factory4 = _dx_com->Factory();
    if (factory4 == nullptr)
        return nullptr;

    DXGI_SWAP_CHAIN_DESC1 swapchain_desc = {0};

    UINT swapchain_flag = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    if (_dx_com->GetFeatures().present_allow_tearing)
        swapchain_flag |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    swapchain_desc.Format = backbufferFormat;
    swapchain_desc.Width = width;
    swapchain_desc.Height = height;
    swapchain_desc.Flags = swapchain_flag;
    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapchain_desc.BufferCount = static_cast<UINT>(swapchainCount);
    swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.SampleDesc.Count = 1;
    swapchain_desc.SampleDesc.Quality = 0;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchain1;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> swapchain3;
    ReturnNullHRFailed(
            factory4->CreateSwapChainForHwnd(
                    commandQueue, hWnd, &swapchain_desc, nullptr, nullptr, swapchain1.GetAddressOf()
            )
    );

    ReturnNullHRFailed(
            factory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER)
    );

    swapchain1.As(&swapchain3);
    return swapchain3;
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> DeviceCom::CreateCommandQueue(
        const D3D12_COMMAND_LIST_TYPE type,
        const D3D12_COMMAND_QUEUE_FLAGS flag,
        const D3D12_COMMAND_QUEUE_PRIORITY priority
) {
    EngineAssert(_dx_com.get());
    ID3D12Device *device = _dx_com->Device();
    if (device == nullptr) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = type;
    queueDesc.Flags = flag;
    queueDesc.Priority = priority;
    ReturnNullHRFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&command_queue)));
    return command_queue;
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> DeviceCom::CreateCommandAllocator(
        const D3D12_COMMAND_LIST_TYPE type
) {
    EngineAssert(_dx_com.get());
    ID3D12Device *device = _dx_com->Device();
    if (device == nullptr) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> command_allocator;
    ReturnNullHRFailed(device->CreateCommandAllocator(
            type,
            IID_PPV_ARGS(&command_allocator))
    );
    return command_allocator;
}


Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> DeviceCom::CreateGraphicsCommandList(
        const D3D12_COMMAND_LIST_TYPE type,
        const D3D12_COMMAND_LIST_FLAGS flags
) {
    EngineAssert(_dx_com.get());
    ID3D12Device4 *device = _dx_com->Device();
    if (device == nullptr) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list;
    ReturnNullHRFailed(
            device->CreateCommandList1(0, type, flags, IID_PPV_ARGS(&command_list))
    );
    return command_list;
}

Microsoft::WRL::ComPtr<ID3D12Fence> DeviceCom::CreateFence(const unsigned __int64 initialValue) {
    EngineAssert(_dx_com.get());
    ID3D12Device *device = _dx_com->Device();
    if (device == nullptr) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    ReturnNullHRFailed(
            device->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()))
    );
    return fence;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DeviceCom::CreateRtvHeap(
        const int descCount,
        const D3D12_DESCRIPTOR_HEAP_FLAGS flag
) {
    EngineAssert(_dx_com.get());
    ID3D12Device *device = _dx_com->Device();
    if (device == nullptr) {
        return nullptr;
    }

    D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_heap;

    rtv_heap_desc.NumDescriptors = descCount;
    rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_heap_desc.Flags = flag;
    rtv_heap_desc.NodeMask = 0;
    ReturnNullHRFailed(device->CreateDescriptorHeap(
            &rtv_heap_desc, IID_PPV_ARGS(rtv_heap.GetAddressOf())
    ));
    return rtv_heap;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DeviceCom::CreateDsvHeap(
        const int descCount,
        const D3D12_DESCRIPTOR_HEAP_FLAGS flag
) {
    EngineAssert(_dx_com.get());
    ID3D12Device *device = _dx_com->Device();
    if (device == nullptr) {
        return nullptr;
    }

    D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {};
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsv_heap;

    dsv_heap_desc.NumDescriptors = descCount;
    dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsv_heap_desc.Flags = flag;
    dsv_heap_desc.NodeMask = 0;
    ReturnNullHRFailed(device->CreateDescriptorHeap(
            &dsv_heap_desc, IID_PPV_ARGS(dsv_heap.GetAddressOf())
    ));
    return dsv_heap;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DeviceCom::CreateSrvCbvUavHeap(
        const int descCount,
        const D3D12_DESCRIPTOR_HEAP_FLAGS flag
) {
    EngineAssert(_dx_com.get());
    ID3D12Device *device = _dx_com->Device();
    if (device == nullptr) {
        return nullptr;
    }

    D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {};
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> scu_heap;

    srv_heap_desc.NumDescriptors = descCount;
    srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srv_heap_desc.Flags = flag;
    ReturnNullHRFailed(device->CreateDescriptorHeap(
            &srv_heap_desc, IID_PPV_ARGS(scu_heap.GetAddressOf())
    ));
    return scu_heap;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DeviceCom::CreateSamplerHeap(
        const int descCount,
        const D3D12_DESCRIPTOR_HEAP_FLAGS flag
) {
    EngineAssert(_dx_com.get());
    ID3D12Device *device = _dx_com->Device();
    if (device == nullptr) {
        return nullptr;
    }

    D3D12_DESCRIPTOR_HEAP_DESC sampler_heap_desc = {};
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> sampler_heap;

    sampler_heap_desc.NumDescriptors = descCount;
    sampler_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
    sampler_heap_desc.Flags = flag;
    ReturnNullHRFailed(device->CreateDescriptorHeap(
            &sampler_heap_desc, IID_PPV_ARGS(sampler_heap.GetAddressOf())
    ));
    return sampler_heap;
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> DeviceCom::CreateRootSignature(
        const CD3DX12_ROOT_PARAMETER1 *parameters,
        const CD3DX12_STATIC_SAMPLER_DESC *staticSamplers,
        const UINT numStaticSampler,
        const UINT numParameter,
        const D3D12_ROOT_SIGNATURE_FLAGS flag
) {
    EngineAssert(_dx_com.get());
    ID3D12Device *device = _dx_com->Device();
    if (device == nullptr) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;
    D3D12_ROOT_SIGNATURE_DESC1 root_sig_desc {
        .NumParameters = numParameter,
        .pParameters = parameters,
        .NumStaticSamplers = numStaticSampler,
        .pStaticSamplers = staticSamplers,
        .Flags = flag
    };
    D3D12_VERSIONED_ROOT_SIGNATURE_DESC desc {
        .Version = D3D_ROOT_SIGNATURE_VERSION_1_1,
        .Desc_1_1 = root_sig_desc
    };

    Microsoft::WRL::ComPtr<ID3DBlob> serialized_root_sig = nullptr;
    HRESULT hr = D3D12SerializeVersionedRootSignature(&desc, serialized_root_sig.GetAddressOf(), nullptr);

//    _com_error err(hr);
//    MessageBox(NULL, err.ErrorMessage(), reinterpret_cast<LPCWSTR>("ASD"), 0);
    ReturnNullHRFailed(hr);
    ReturnNullHRFailed(device->CreateRootSignature(
            0,
            serialized_root_sig->GetBufferPointer(),
            serialized_root_sig->GetBufferSize(),
            IID_PPV_ARGS(root_signature.GetAddressOf())
    ));

    return root_signature;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DeviceCom::CreateResource(
        CD3DX12_HEAP_PROPERTIES *heapProperties,
        CD3DX12_RESOURCE_DESC *resourceDesc,
        CD3DX12_CLEAR_VALUE *clearValue,
        const D3D12_HEAP_FLAGS heapFlags,
        const D3D12_RESOURCE_STATES resourceState
) {
    EngineAssert(_dx_com.get());
    ID3D12Device *device = _dx_com->Device();
    if (device == nullptr) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    ReturnNullHRFailed(device->CreateCommittedResource(
            heapProperties, heapFlags, resourceDesc, resourceState, clearValue, IID_PPV_ARGS(resource.GetAddressOf())
    ));
    return resource;
}

Microsoft::WRL::ComPtr<ID3D12CommandQueue> DeviceCom::GetCommandQueue() {
    return _command_queue;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DeviceCom::GetCurrentBackBuffer() {
    EngineAssert(_swapchain.Get() != nullptr);
    int idx = _swapchain->GetCurrentBackBufferIndex();
    return GetBackBuffer(idx);
}

UINT DeviceCom::GetCurrentBackBufferIndex() {
    return _swapchain->GetCurrentBackBufferIndex();
}

Microsoft::WRL::ComPtr<ID3D12Resource> DeviceCom::GetBackBuffer(int idx) {
    EngineAssert(_swapchain.Get() != nullptr);
    Microsoft::WRL::ComPtr<ID3D12Resource> backbuffer;
    ReturnNullHRFailed(_swapchain->GetBuffer(idx, IID_PPV_ARGS(backbuffer.GetAddressOf())));

    return backbuffer;
}

bool DeviceCom::Present(bool VSync) {
    EngineAssert(_swapchain.Get() != nullptr);

    UINT interval = 0u;
    if (VSync) interval = 1u;
    UINT flags = (_dx_com->GetFeatures().present_allow_tearing && !VSync) ? DXGI_PRESENT_ALLOW_TEARING : 0u;

    ReturnFalseHRFailed(_swapchain->Present(interval, flags));
    return true;
}

bool DeviceCom::ResizeSwapchain(const UINT width, const UINT height) {
    EngineAssert(_swapchain.Get() != nullptr);
    DXGI_SWAP_CHAIN_DESC1 desc;
    _swapchain->GetDesc1(&desc);
    ReturnFalseHRFailed(_swapchain->ResizeBuffers(desc.BufferCount, width, height, desc.Format, desc.Flags));
    return true;
}

bool DeviceCom::LoadDxCom() {
    auto dx = std::make_unique<DXCom>();
    if (!dx->Initiate())
        return false;

    _dx_com = std::move(dx);
    return true;
}

ID3D12Device5 *DeviceCom::GetDevice() const {
    return _dx_com->Device();
}

void DeviceCom::Close() {
    _dx_com->Close();
}

Microsoft::WRL::ComPtr<ID3D12PipelineState>
DeviceCom::CreatePipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC *desc) {
    EngineAssert(_dx_com.get());
    ID3D12Device *device = _dx_com->Device();
    if (device == nullptr) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;

    ReturnNullHRFailed(device->CreateGraphicsPipelineState(
            desc, IID_PPV_ARGS(pso.GetAddressOf())
    ));
    return pso;
}

bool DeviceCom::CreateSamplerResource(D3D12_SAMPLER_DESC *desc, D3D12_CPU_DESCRIPTOR_HANDLE handle) {
    EngineAssert(_dx_com.get());
    ID3D12Device *device = _dx_com->Device();
    if (device == nullptr) {
        return false;
    }

    device->CreateSampler(desc, handle);
    return true;
}
_END_ENGINE
