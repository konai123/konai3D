//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/upload_buffer.h"

_START_ENGINE
UploadBuffer::UploadBuffer(DeviceCom *device, int elementCount, UINT elementSize, bool isConstant)
:
_mapped_data(nullptr),
_byte_size(0) {
    _byte_size = isConstant ? GetConstantBufferBytes(elementCount, elementSize) : elementSize * elementCount;
    _element_byte_size = isConstant ? GetConstantBufferBytes(1, elementSize) : elementSize;

    auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resource_desc = CD3DX12_RESOURCE_DESC::Buffer(_byte_size);
    _upload_buffer = device->CreateResource(&properties,
                                            &resource_desc,
                                            nullptr,
                                            D3D12_HEAP_FLAG_NONE,
                                            D3D12_RESOURCE_STATE_GENERIC_READ);
    EngineAssert(_upload_buffer != nullptr);
    _upload_buffer->Map(0, nullptr, reinterpret_cast<void **>(&_mapped_data));
}

UploadBuffer::~UploadBuffer() {
    _upload_buffer->Unmap(0, nullptr);
    _mapped_data = nullptr;
}

ID3D12Resource *UploadBuffer::Resource() {
    return _upload_buffer.Get();
}

void UploadBuffer::Copy(int elementIndex, void *data) {
    memcpy(_mapped_data + (elementIndex * _element_byte_size), data, _element_byte_size);
}

_END_ENGINE