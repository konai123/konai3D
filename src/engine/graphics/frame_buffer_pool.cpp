//
// Created by khhan on 2021-06-16.
//

#include "src/engine/graphics/frame_buffer_pool.h"

_START_ENGINE
FrameBufferPool::FrameBufferPool()
:
_commandQueue(nullptr),
_fence(nullptr),
_fence_value(1),
_frame_count(0),
_current_frame_idx(0),
_type(D3D12_COMMAND_LIST_TYPE_DIRECT) {}

bool FrameBufferPool::Initiate(
        DeviceCom *device,
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue,
        const UINT frameCount,
        const D3D12_COMMAND_LIST_TYPE type
) {
    EngineAssert(device != nullptr);
    _commandQueue = commandQueue;
    _fence = device->CreateFence(_fence_value);
    _type = type;

    if (_fence == nullptr) {
        GRAPHICS_LOG_ERROR("Failed to create a fence.");
        return false;
    }

    _frame_count = frameCount;
    for (UINT i = 0; i < _frame_count; i++) {
        _frame_buffers.emplace_back(device, _type);
    }
    return true;
}

FrameBuffer *FrameBufferPool::RequestFrameBuffer(DeviceCom *device) {
    UINT64 fence_value = _frame_buffers[_current_frame_idx]._fence_value;
    UINT64 current_fence = _fence->GetCompletedValue();

    if (fence_value != 0 && current_fence < fence_value) {
        HANDLE event = ::CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        if (event == INVALID_HANDLE_VALUE) {
            EngineAssert(false);
        }

        ThrowHRFailed(_fence->SetEventOnCompletion(fence_value, event));

        ::WaitForSingleObject(event, INFINITE);
        ::CloseHandle(event);
    }

    UINT ret_idx = _current_frame_idx;
    _current_frame_idx = (ret_idx + 1) % _frame_count;

    return &_frame_buffers[ret_idx];
}

void FrameBufferPool::DiscardFrameBuffer(FrameBuffer *frameBuffer) {
    EngineAssert(_commandQueue.Get() != nullptr);

    _fence_value++;
    _commandQueue->Signal(_fence.Get(), _fence_value);
    frameBuffer->_fence_value = _fence_value;
}

void FrameBufferPool::WaitAllExecute() {

    EngineAssert(_commandQueue.Get() != nullptr);
    EngineAssert(_fence.Get() != nullptr);

    if (_fence->GetCompletedValue() < _fence_value) {
        HANDLE event = ::CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        if (event == INVALID_HANDLE_VALUE) {
            EngineAssert(false);
        }

        ThrowHRFailed(_fence->SetEventOnCompletion(_fence_value, event));

        ::WaitForSingleObject(event, INFINITE);
        ::CloseHandle(event);
    }
}
_END_ENGINE
