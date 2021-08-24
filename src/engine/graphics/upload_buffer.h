//
// Created by korona on 2021-06-20.
//

#pragma once

#include "src/engine/graphics/utils.hpp"

_START_ENGINE
class UploadBuffer {
public:
    UploadBuffer(DeviceCom *device, int elementCount, UINT elementSize, bool isConstant);
    UploadBuffer(const UploadBuffer &) = delete;
    UploadBuffer &operator=(const UploadBuffer &) = delete;
    virtual ~UploadBuffer();

public:
    ID3D12Resource *Resource();
    void Copy(int elementIndex, void *data);
    void SafeRelease();

public:
    unsigned char *MappedData;
    size_t ByteSize;
    size_t ElementByteSize;

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> _upload_buffer;
};
_END_ENGINE
