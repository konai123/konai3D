//
// Created by korona on 2021-08-04.
//

#include "src/engine/graphics/rw_resource_buffer.h"

_START_ENGINE
RWResourceBuffer::RWResourceBuffer(std::shared_ptr<DeviceCom> device, UINT numPreFrames)
:
_device(device),
_num_pre_frames(numPreFrames) {}

ID3D12Resource *RWResourceBuffer::GetResource(UINT currentFrameIdx) {
    return _cb_buffers[currentFrameIdx]->Resource();
}

bool RWResourceBuffer::SetData(void *initData, UINT numResource, UINT databytesSize) {
    for (UINT i = 0; i < _num_pre_frames; i++) {
        auto upbuffer = std::make_unique<UploadBuffer>(_device.get(), numResource, databytesSize, false);
        for (UINT j = 0; j < numResource; j++) {
            upbuffer->Copy(j, initData);
        }
        _cb_buffers.push_back(std::move(upbuffer));
    }
    _bytes_size = numResource * databytesSize;
    return true;
}

bool RWResourceBuffer::SetData(UINT numResource, UINT bytesSize) {
    for (UINT i = 0; i < _num_pre_frames; i++) {
        auto upbuffer = std::make_unique<UploadBuffer>(_device.get(), numResource, bytesSize, false);
        _cb_buffers.push_back(std::move(upbuffer));
    }
    _bytes_size = numResource * bytesSize;
    return true;
}

bool RWResourceBuffer::UpdateData(void *data, UINT resourceIdx, UINT currentFrameIdx) {
    if (_cb_buffers[currentFrameIdx] == nullptr) {
        GRAPHICS_LOG_ERROR("Invalid resource buffers.");
        return false;
    }
    _cb_buffers[currentFrameIdx]->Copy(resourceIdx, data);
    return true;
}

UINT RWResourceBuffer::GetResourceBytesSize() {
    return _bytes_size;
}


void RWResourceBuffer::SafeRelease() {
    for (UINT i = 0; i < _num_pre_frames; i++) {
        _cb_buffers[i]->SafeRelease();
        _cb_buffers[i] = nullptr;
    }
}

_END_ENGINE
