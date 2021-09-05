//
// Created by korona on 2021-06-08.
//

#include "src/engine/graphics/dx_com.h"
#include "src/engine/graphics/macros.h"

extern "C" {
    _declspec(dllexport)
    extern const UINT D3D12SDKVersion = 4;
}
extern "C" {
    _declspec(dllexport)
    extern const char *D3D12SDKPath = ".\\D3D12\\";
}

_START_ENGINE
DXCom::DXCom()
:
_dxgi_factory(nullptr),
_dxgi_device(nullptr)
{}

bool DXCom::Initiate() {
#if defined(DEBUG) || defined(_DEBUG)
    Microsoft::WRL::ComPtr<ID3D12Debug1> debug1;
    if (SUCCEEDED(::D3D12GetDebugInterface(IID_PPV_ARGS(debug1.ReleaseAndGetAddressOf())))) {
        debug1->EnableDebugLayer();
        debug1->SetEnableGPUBasedValidation(true);
    }
#endif
    ReturnFalseHRFailed(::CreateDXGIFactory1(IID_PPV_ARGS(&_dxgi_factory)));
    if (!PrepareDevice()) {
        return false;
    }

#if defined(DEBUG) || defined(_DEBUG)
    Microsoft::WRL::ComPtr<ID3D12InfoQueue> info_queue;
    if (SUCCEEDED(_dxgi_device.As(&info_queue))) {
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
    }
#endif

    //Check DXGI_FEATURE_PRESENT_ALLOW_TEARING
    Microsoft::WRL::ComPtr<IDXGIFactory5> factory5;
    HRESULT hr = _dxgi_factory.As(&factory5);
    BOOL allowTearing = FALSE;
    if (SUCCEEDED(hr)) {
        if (SUCCEEDED(hr)) {
            hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
        }
    }
    _features.present_allow_tearing = SUCCEEDED(hr) && allowTearing;
    return true;
}

ID3D12Device5 *DXCom::Device() const {
    return _dxgi_device.Get();
}

IDXGIFactory4 *DXCom::Factory() const {
    return _dxgi_factory.Get();
}

bool DXCom::PrepareDevice() {
    if (_dxgi_factory) {
        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;

        //find max video memory adapter.
        int adapter_index = -1;
        {
            int i = 0;
            size_t mem_size = 0;
            auto adapters = GetGraphicsAdapters(_dxgi_factory.Get());
            for (auto adp : adapters) {
                DXGI_ADAPTER_DESC adapterDesc;
                adp->GetDesc(&adapterDesc);
                size_t vm_size = adapterDesc.DedicatedVideoMemory;

                GRAPHICS_LOG_DEBUG("{} -- v_memory_size(MB): {}", fmt::ptr(adapterDesc.Description),
                               vm_size / 1024 / 1024);

                if (vm_size > mem_size || adapter_index == -1) {
                    mem_size = vm_size;
                    adapter_index = i;
                    adapter = adp;
                }
                i++;
            }
        }

        if (adapter_index == -1) {
            return false;
        }

        HRESULT hr = D3D12CreateDevice(
                adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&_dxgi_device)
        );

        if (SUCCEEDED(hr)) {
            GRAPHICS_LOG_INFO("using Primary Adapter.");
            GRAPHICS_LOG_INFO("checking raytracing feature...");
            D3D12_FEATURE_DATA_D3D12_OPTIONS5 features = {};
            _dxgi_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features, sizeof(features));
            if (features.RaytracingTier < D3D12_RAYTRACING_TIER_1_0) {
                GRAPHICS_LOG_ERROR("feature check failed");
                return false;
            }
            GRAPHICS_LOG_INFO("feature check succeed : [D3D12_RAYTRACING_TIER: {}]", features.RaytracingTier);
            return true;
        }
        return false;
    }
    return false;
}

/*
* Return Adapter list
* */
std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter>> DXCom::GetGraphicsAdapters(IDXGIFactory *factory) {
    std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter>> ret;

    if (factory) {
        IDXGIAdapter *adapter = nullptr;
        for (int i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
            ret.emplace_back(adapter);
        }
    }
    return ret;
}

void DXCom::Close() {

}

DXGIFeature DXCom::GetFeatures() const {
    return _features;
}
_END_ENGINE