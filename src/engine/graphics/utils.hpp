//
// Created by korona on 2021-06-20.
//

#ifndef KONAI3D_UTILS_HPP
#define KONAI3D_UTILS_HPP

#include "src/engine/graphics/device_com.h"
#include "src/engine/core/macros.h"

_START_ENGINE
namespace {
bool UploadResource(
        DeviceCom *device,
        ID3D12GraphicsCommandList *cmdList,
        void *initData,
        size_t byteSize,
        Microsoft::WRL::ComPtr<ID3D12Resource> &outUploadBuffer,
        Microsoft::WRL::ComPtr<ID3D12Resource> &outDefaultBuffer
) {
    outUploadBuffer = nullptr;
    outDefaultBuffer = nullptr;

    auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resource_desc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    outUploadBuffer = device->CreateResource(
            &properties,
            &resource_desc,
            nullptr,
            D3D12_HEAP_FLAG_NONE,
            D3D12_RESOURCE_STATE_GENERIC_READ
    );
    if (outUploadBuffer == nullptr)
        return false;

    properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    resource_desc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    outDefaultBuffer = device->CreateResource(
            &properties,
            &resource_desc,
            nullptr,
            D3D12_HEAP_FLAG_NONE,
            D3D12_RESOURCE_STATE_COMMON
    );

    if (outDefaultBuffer == nullptr) {
        outUploadBuffer = nullptr;
        return false;
    }

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(outDefaultBuffer.Get(), D3D12_RESOURCE_STATE_COMMON,
                                                        D3D12_RESOURCE_STATE_COPY_DEST);
    cmdList->ResourceBarrier(1, &barrier);

    D3D12_SUBRESOURCE_DATA subresource;
    subresource.pData = initData;
    subresource.RowPitch = byteSize;
    subresource.SlicePitch = subresource.RowPitch;
    UpdateSubresources<1>(cmdList, outDefaultBuffer.Get(), outUploadBuffer.Get(), 0, 0, 1, &subresource);

    barrier = CD3DX12_RESOURCE_BARRIER::Transition(outDefaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                                                   D3D12_RESOURCE_STATE_GENERIC_READ);
    cmdList->ResourceBarrier(1, &barrier);
    return true;
}

unsigned int GetConstantBufferBytes(int elementCount, int elementSize) {
    int original_size = elementSize;
    return ((original_size + 255) & ~255) * elementCount;
}

} //end namespace
_END_ENGINE
#endif //KONAI3D_UTILS_HPP
