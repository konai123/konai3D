//
// Created by korona on 2021-09-20.
//

#pragma once

#include "src/engine/graphics/device_com.h"

_START_ENGINE
class ToneMapper{
public:
    explicit ToneMapper(std::shared_ptr<DeviceCom> deviceCom);

public:
    bool Initiate();

    void Render(float delta,
                ID3D12GraphicsCommandList6 *command_list,
                RenderScreen* screen,
                HeapDescriptorHandle* shaderResourceView
    );

private:
    bool BuildRootSignature();
    bool BuildPSO();

private:
    std::shared_ptr<DeviceCom> _device_com;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> _root_signature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> _pso;
};
_END_ENGINE
