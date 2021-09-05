//
// Created by korona on 2021-08-26.
//

#pragma once

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/upload_buffer.h"

_START_ENGINE
class FrameResourceBuffer {
public:
    FrameResourceBuffer(std::shared_ptr<DeviceCom> device, UINT numPreFrames);

public:
    virtual bool Initialize(CD3DX12_RESOURCE_DESC desc);
    virtual bool UpdateData(void *data, UINT currentFrameIdx, ID3D12GraphicsCommandList* cmdList);
    virtual UINT GetResourceBytesSize();
    virtual void ResourceBarrier(D3D12_RESOURCE_STATES toState, UINT currentFrameIdx, ID3D12GraphicsCommandList* cmdList);
    virtual void SafeRelease();

public:
    virtual ID3D12Resource *GetResource(UINT currentFrameIdx);

protected:
    std::shared_ptr<DeviceCom> _device;
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _upload_buffer;
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> _resource;
    std::vector<D3D12_RESOURCE_STATES> _state;
    UINT _num_pre_frames;
    UINT _bytes_size;
};
_END_ENGINE
