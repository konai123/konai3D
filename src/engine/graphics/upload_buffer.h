//
// Created by korona on 2021-06-20.
//

#ifndef KONAI3D_UPLOAD_BUFFER_H
#define KONAI3D_UPLOAD_BUFFER_H

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

public:
    unsigned char *_mapped_data;
    size_t _byte_size;
    size_t _element_byte_size;

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> _upload_buffer;
};
_END_ENGINE
#endif //KONAI3D_UPLOAD_BUFFER_H
