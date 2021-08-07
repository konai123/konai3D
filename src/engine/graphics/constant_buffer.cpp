//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/constant_buffer.h"

_START_ENGINE
ConstantBuffer::ConstantBuffer(std::shared_ptr <DeviceCom> device, UINT numPreFrames)
:
_device(device),
_num_pre_frames(numPreFrames) {}

ID3D12Resource *ConstantBuffer::GetResource(UINT currentFrameIdx) {
    return _cb_buffers[currentFrameIdx]->Resource();
}

bool ConstantBuffer::SetData(void *initData, UINT bytesSize) {
    for (UINT i = 0; i < _num_pre_frames; i++) {
        auto upbuffer = std::make_unique<UploadBuffer>(_device.get(), 1, bytesSize, true);
        upbuffer->Copy(0, initData);
        _cb_buffers.push_back(std::move(upbuffer));
    }
    return true;
}

bool ConstantBuffer::UpdateData(void *data, UINT currentFrameIdx) {
    if (_cb_buffers[currentFrameIdx] == nullptr) {
        GRAPHICS_LOG_ERROR("invalid constante buffers.");
        return false;
    }
    _cb_buffers[currentFrameIdx]->Copy(0, data);
    return true;
}

_END_ENGINE