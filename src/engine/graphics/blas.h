//
// Created by korona on 2021-08-19.
//


#pragma once

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/macros.h"

_START_ENGINE
struct BLAS {
    bool Generate(DeviceCom* deviceCom, ID3D12GraphicsCommandList5* cmdList);
    void AddGeometry(D3D12_RAYTRACING_GEOMETRY_DESC& geoDesc);
    UINT GetSize();

    std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> GeometryDescs;
    Microsoft::WRL::ComPtr<ID3D12Resource> ScratchBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> ResultDataBuffer;
};
_END_ENGINE
