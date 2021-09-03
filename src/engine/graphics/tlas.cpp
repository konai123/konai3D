//
// Created by korona on 2021-08-19.
//

#include "src/engine/graphics/tlas.h"
#include "src/engine/graphics/macros.h"
#include "src/engine/graphics/resource_garbage_queue.h"

_START_ENGINE
bool TLAS::Initialize(UINT maxInstance, DeviceCom *deviceCom) {
    auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC resource_desc = CD3DX12_RESOURCE_DESC::Buffer(
            sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * maxInstance,
            D3D12_RESOURCE_FLAG_NONE,
            0
    );
    InstanceDescsResource = deviceCom->CreateResource(
            &properties,
            &resource_desc,
            nullptr,
            D3D12_HEAP_FLAG_NONE,
            D3D12_RESOURCE_STATE_GENERIC_READ
    );
    if (InstanceDescsResource == nullptr) {
        GRAPHICS_LOG_ERROR("Failed to create resource");
        return false;
    }

    return true;
}

bool TLAS::Generate(DeviceCom *deviceCom, ID3D12GraphicsCommandList5 *cmdList) {
    {
        UINT8 *p_data;
        EngineAssert(InstanceDescsResource != nullptr);
        ReturnFalseHRFailed(InstanceDescsResource->Map(0, nullptr, reinterpret_cast<void **>(&p_data)));
        ::memcpy(p_data, InstanceDescs.data(), InstanceDescs.size() * sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
        InstanceDescsResource->Unmap(0, nullptr);
    }

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS as_input = {};
    as_input.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
    as_input.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    as_input.InstanceDescs = InstanceDescsResource->GetGPUVirtualAddress();
    as_input.NumDescs = InstanceDescs.size();
    as_input.Flags = buildFlags;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild_info;
    deviceCom->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&as_input, &prebuild_info);

    prebuild_info.ScratchDataSizeInBytes =
            ALIGN(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT, prebuild_info.ScratchDataSizeInBytes);
    prebuild_info.ResultDataMaxSizeInBytes =
            ALIGN(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT, prebuild_info.ResultDataMaxSizeInBytes);

    auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    if (ScratchBuffer == nullptr || ScratchBuffer->GetDesc().Width < prebuild_info.ScratchDataSizeInBytes) {
        CD3DX12_RESOURCE_DESC scratch_buf_desc =
                CD3DX12_RESOURCE_DESC::Buffer(prebuild_info.ScratchDataSizeInBytes,
                                              D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

        if (ScratchBuffer != nullptr) {
            ResourceGarbageQueue::Instance().SubmitResource(ScratchBuffer);
            ScratchBuffer = nullptr;
        }

        ScratchBuffer = deviceCom->CreateResource(
                &properties,
                &scratch_buf_desc,
                nullptr,
                D3D12_HEAP_FLAG_NONE,
                D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        if (ScratchBuffer == nullptr) {
            return false;
        }
    }

    if (ResultDataBuffer == nullptr || ResultDataBuffer->GetDesc().Width < prebuild_info.ResultDataMaxSizeInBytes) {
        CD3DX12_RESOURCE_DESC resoult_buf_desc =
                CD3DX12_RESOURCE_DESC::Buffer(prebuild_info.ResultDataMaxSizeInBytes,
                                              D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

        if (ResultDataBuffer != nullptr) {
            ResourceGarbageQueue::Instance().SubmitResource(ResultDataBuffer);
            ResultDataBuffer = nullptr;
        }

        ResultDataBuffer = deviceCom->CreateResource(
                &properties,
                &resoult_buf_desc,
                nullptr,
                D3D12_HEAP_FLAG_NONE,
                D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);

        if (ResultDataBuffer == nullptr) {
            return false;
        }
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

void TLAS::AddInstance(D3D12_RAYTRACING_INSTANCE_DESC &instDesc) {
    InstanceDescs.push_back(instDesc);
}

void TLAS::Clear() {
    InstanceDescs.clear();
}

UINT TLAS::Size() {
    return InstanceDescs.size();
}
_END_ENGINE
