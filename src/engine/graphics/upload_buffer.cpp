//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/upload_buffer.h"
#include "src/engine/graphics/resource_garbage_queue.h"

_START_ENGINE
UploadBuffer::UploadBuffer(DeviceCom *device, int elementCount, UINT elementSize, bool isConstant)
:
MappedData(nullptr),
ByteSize(0) {
    ByteSize = isConstant ? GetConstantBufferBytes(elementCount, elementSize) : elementSize * elementCount;
    ElementByteSize = isConstant ? GetConstantBufferBytes(1, elementSize) : elementSize;

    auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resource_desc = CD3DX12_RESOURCE_DESC::Buffer(ByteSize);
    _upload_buffer = device->CreateResource(&properties,
                                            &resource_desc,
                                            nullptr,
                                            D3D12_HEAP_FLAG_NONE,
                                            D3D12_RESOURCE_STATE_GENERIC_READ);
    EngineAssert(_upload_buffer != nullptr);
    _upload_buffer->Map(0, nullptr, reinterpret_cast<void **>(&MappedData));
}

UploadBuffer::~UploadBuffer() {
    _upload_buffer->Unmap(0, nullptr);
    MappedData = nullptr;
}

ID3D12Resource *UploadBuffer::Resource() {
    return _upload_buffer.Get();
}

void UploadBuffer::Copy(int elementIndex, void *data) {
    memcpy(MappedData + (elementIndex * ElementByteSize), data, ElementByteSize);
}

void UploadBuffer::SafeRelease() {
    ResourceGarbageQueue::Instance().SubmitResource(_upload_buffer);
}

_END_ENGINE