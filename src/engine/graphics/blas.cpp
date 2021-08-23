//
// Created by korona on 2021-08-19.
//

#include "src/engine/graphics/blas.h"

_START_ENGINE
bool BLAS::Generate(DeviceCom *deviceCom, ID3D12GraphicsCommandList5* cmdList) {
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS build_flag;
    build_flag = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS as_input;
    as_input.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    as_input.Flags = build_flag;
    as_input.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    as_input.NumDescs = static_cast<UINT>(GeometryDescs.size());
    as_input.pGeometryDescs = GeometryDescs.data();

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild_info;
    deviceCom->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&as_input, &prebuild_info);

    prebuild_info.ScratchDataSizeInBytes =
            ALIGN(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT, prebuild_info.ScratchDataSizeInBytes);
    prebuild_info.ResultDataMaxSizeInBytes =
            ALIGN(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT, prebuild_info.ResultDataMaxSizeInBytes);

    CD3DX12_RESOURCE_DESC scratch_buf_desc =
            CD3DX12_RESOURCE_DESC::Buffer(prebuild_info.ScratchDataSizeInBytes,
                                          D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    ScratchBuffer = deviceCom->CreateResource(
            &properties,
            &scratch_buf_desc,
            nullptr,
            D3D12_HEAP_FLAG_NONE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    if (ScratchBuffer == nullptr) {
        return false;
    }

    ResultDataBuffer = deviceCom->CreateResource(
            &properties,
            &scratch_buf_desc,
            nullptr,
            D3D12_HEAP_FLAG_NONE,
            D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);

    if (ResultDataBuffer == nullptr) {
        ScratchBuffer.Reset();
        return false;
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build_desc = {};
    build_desc.Inputs = as_input;
    build_desc.ScratchAccelerationStructureData = ScratchBuffer->GetGPUVirtualAddress();
    build_desc.DestAccelerationStructureData = ResultDataBuffer->GetGPUVirtualAddress();

    cmdList->BuildRaytracingAccelerationStructure(&build_desc, 0, nullptr);
    D3D12_RESOURCE_BARRIER uav_barrier;
    uav_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    uav_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    uav_barrier.UAV.pResource = ResultDataBuffer.Get();
    cmdList->ResourceBarrier(1, &uav_barrier);
    return true;
}

void BLAS::AddGeometry(D3D12_RAYTRACING_GEOMETRY_DESC &geoDesc) {
    GeometryDescs.push_back(geoDesc);
}
_END_ENGINE
