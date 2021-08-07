//
// Created by korona on 2021-08-04.
//

#pragma once

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/upload_buffer.h"

_START_ENGINE

class RWResourceBuffer {
public:
    RWResourceBuffer(std::shared_ptr<DeviceCom> device, UINT numPreFrames);

public:
    ID3D12Resource *GetResource(UINT currentFrameIdx);
    bool SetData(void *initData, UINT numResource, UINT bytesSize);
    bool UpdateData(void *data, UINT resourceIdx, UINT currentFrameIdx);

private:
    std::shared_ptr<DeviceCom> _device;
    std::vector<std::unique_ptr<UploadBuffer>> _cb_buffers;
    UINT _num_pre_frames;
};

_END_ENGINE
