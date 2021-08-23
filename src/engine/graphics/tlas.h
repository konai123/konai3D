//
// Created by korona on 2021-08-19.
//

#pragma once

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/upload_buffer.h"

_START_ENGINE
struct TLAS {
    bool Generate(DeviceCom* deviceCom, ID3D12GraphicsCommandList5* cmdList);
    void AddInstance(D3D12_RAYTRACING_INSTANCE_DESC& instDesc);
    void Clear();

    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> InstanceDescs;
    Microsoft::WRL::ComPtr<ID3D12Resource>  InstanceDescsResource;
    Microsoft::WRL::ComPtr<ID3D12Resource> ScratchBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> ResultDataBuffer;
};
_END_ENGINE