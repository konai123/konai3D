//
// Created by korona on 2021-08-26.
//

#include "src/engine/graphics/frame_resource_buffer.h"
#include "src/engine/graphics/resource_garbage_queue.h"

_START_ENGINE
FrameResourceBuffer::FrameResourceBuffer(std::shared_ptr<DeviceCom> device, UINT numPreFrames)
:
_device(device),
_num_pre_frames(numPreFrames) {}

bool FrameResourceBuffer::Initialize(CD3DX12_RESOURCE_DESC desc) {
    for (UINT i = 0; i < _num_pre_frames; i++) {
        auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto resource = _device->CreateResource(&properties, &desc, nullptr, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON);

        if (resource == nullptr) {
            GRAPHICS_LOG_ERROR("Cannot create resource");
            return false;
        }
        _state.push_back(D3D12_RESOURCE_STATE_COMMON);
        _resource.push_back(resource);
    }

    auto upload_desc = CD3DX12_RESOURCE_DESC::Buffer(desc.Width);
    auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    _upload_buffer = _device->CreateResource(
            &properties,
            &upload_desc,
            nullptr,
            D3D12_HEAP_FLAG_NONE,
            D3D12_RESOURCE_STATE_GENERIC_READ
    );
    if (_upload_buffer == nullptr)
        return false;

    _bytes_size = desc.Width;
    return true;
}

bool FrameResourceBuffer::UpdateData(void *data, UINT currentFrameIdx, ID3D12GraphicsCommandList* cmdList) {
    auto current_state = _state[currentFrameIdx];

    ResourceBarrier(D3D12_RESOURCE_STATE_COPY_DEST, currentFrameIdx, cmdList);

    D3D12_SUBRESOURCE_DATA subresource;
    subresource.pData = data;
    subresource.RowPitch = GetResourceBytesSize();
    subresource.SlicePitch = subresource.RowPitch;
    ::UpdateSubresources<1>(cmdList, _resource[currentFrameIdx].Get(), _upload_buffer.Get(), 0, 0, 1, &subresource);

    ResourceBarrier(current_state, currentFrameIdx, cmdList);

    return true;
}

UINT FrameResourceBuffer::GetResourceBytesSize() {
    return _bytes_size;
}

void FrameResourceBuffer::ResourceBarrier(D3D12_RESOURCE_STATES toState, UINT currentFrameIdx, ID3D12GraphicsCommandList* cmdList) {
    auto resource_barrier_enter = CD3DX12_RESOURCE_BARRIER::Transition(
            _resource[currentFrameIdx].Get(),
            _state[currentFrameIdx],
            toState
    );

    cmdList->ResourceBarrier(1, &resource_barrier_enter);
    _state[currentFrameIdx] = toState;
}

void FrameResourceBuffer::SafeRelease() {
    for (UINT i = 0; i < _num_pre_frames; i++) {
        ResourceGarbageQueue::Instance().SubmitResource(_resource[i]);
        _resource[i] = nullptr;
    }
    ResourceGarbageQueue::Instance().SubmitResource(_upload_buffer);
    _upload_buffer = nullptr;
}

ID3D12Resource *FrameResourceBuffer::GetResource(UINT currentFrameIdx) {
    return _resource[currentFrameIdx].Get();
}
_END_ENGINE
