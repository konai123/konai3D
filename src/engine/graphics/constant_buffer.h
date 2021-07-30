//
// Created by korona on 2021-06-28.
//

#ifndef KONAI3D_CONSTANT_BUFFER_H
#define KONAI3D_CONSTANT_BUFFER_H

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/upload_buffer.h"

_START_ENGINE
class ConstantBuffer {
public:
    ConstantBuffer(std::shared_ptr<DeviceCom> device, UINT numPreFrames);

public:
    ID3D12Resource *GetResource(UINT currentFrameIdx);
    bool SetData(void *initData, UINT bytesSize);
    bool UpdateData(void *data, UINT currentFrameIdx);

private:
    std::shared_ptr<DeviceCom> _device;
    std::vector<std::unique_ptr<UploadBuffer>> _cb_buffers;
    UINT _num_pre_frames;
};
_END_ENGINE
#endif //KONAI3D_CONSTANT_BUFFER_H
