//
// Created by korona on 2021-06-08.
//

#pragma once

_START_ENGINE
struct DXGIFeature {
    bool present_allow_tearing;
};

class DXCom {
public:
    DXCom();
    virtual ~DXCom() = default;
    DXCom &operator=(const DXCom &) = delete;
    DXCom &operator=(DXCom &&) noexcept = default;
    DXCom(const DXCom &) = delete;
    DXCom(DXCom &&) noexcept = default;

public:
    virtual bool Initiate();
    virtual void Close();

public:
    ID3D12Device5 *Device() const;
    IDXGIFactory4 *Factory() const;
    DXGIFeature GetFeatures() const;

public:
    static std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter>> GetGraphicsAdapters(IDXGIFactory *factory);

private:
    bool PrepareDevice();

private:
    Microsoft::WRL::ComPtr<IDXGIFactory4> _dxgi_factory;
    Microsoft::WRL::ComPtr<ID3D12Device5> _dxgi_device;

private:
    DXGIFeature _features;
};
_END_ENGINE

