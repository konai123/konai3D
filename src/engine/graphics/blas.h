//
// Created by korona on 2021-08-19.
//


#pragma once

#include "src/engine/graphics/mesh_map.h"

_START_ENGINE
struct BLAS {
    bool Generate(DeviceCom* deviceCom, ID3D12GraphicsCommandList5* cmdList);
    void AddGeometry(D3D12_RAYTRACING_GEOMETRY_DESC& geoDesc);

    std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> GeometryDescs;
    Microsoft::WRL::ComPtr<ID3D12Resource> scratch_buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> result_data_buffer;
};
_END_ENGINE
