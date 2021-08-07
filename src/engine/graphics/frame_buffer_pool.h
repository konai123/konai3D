//
// Created by khhan on 2021-06-16.
//

#pragma once

#include "src/engine/graphics/device_com.h"

_START_ENGINE
/*
 * No Free Thread.
 * */
struct FrameBuffer {
public:
    FrameBuffer(DeviceCom *device, D3D12_COMMAND_LIST_TYPE type)
    :
    _fence_value(1) {
        _allocator = device->CreateCommandAllocator(type);
    }

public:
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _allocator;
    UINT64 _fence_value;
};

class FrameBufferPool {
public:
    FrameBufferPool();
    virtual ~FrameBufferPool() = default;
    FrameBufferPool &operator=(const FrameBufferPool &) = delete;
    FrameBufferPool &operator=(FrameBufferPool &&) noexcept = default;
    FrameBufferPool(const FrameBufferPool &) = delete;
    FrameBufferPool(FrameBufferPool &&) noexcept = default;

public:
    virtual bool Initiate(
            DeviceCom *device,
            Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
            const UINT frameCount = 3,
            const D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT
    );
    virtual FrameBuffer *RequestFrameBuffer(DeviceCom *device);
    virtual void DiscardFrameBuffer(FrameBuffer *frameBuffer);
    virtual void WaitAllExecute();

private:
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> _commandQueue;
    Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
    std::vector<FrameBuffer> _frame_buffers;

private:
    UINT _current_frame_idx;
    UINT _frame_count;
    UINT64 _fence_value;
    D3D12_COMMAND_LIST_TYPE _type;
};
_END_ENGINE
