//
// Created by korona on 2021-06-20.
//

#include "src/engine/graphics/vertex_buffer.h"

_START_ENGINE
VertexBuffer::VertexBuffer(
        DeviceCom *device,
        ID3D12GraphicsCommandList *cmd_list,
        void *vertices,
        UINT numVertex,
        UINT vertexSize,
        UINT32 *indices,
        UINT numIndex,
        bool isDynamicBuffer
)
:
_dynamic_buffer(nullptr),
_vertex_upload_buffer(nullptr),
_index_upload_buffer(nullptr),
_vertex_buffer(nullptr),
_index_buffer(nullptr),
_isDynamicBuffer(isDynamicBuffer) {
    EngineAssert(device != nullptr);

    _index_buffer_byte_size = numIndex * sizeof(UINT32);
    _vertex_buffer_byte_size = numVertex * vertexSize;

    if (!isDynamicBuffer) {
        if (!UploadResource(device, cmd_list, vertices, _vertex_buffer_byte_size, _vertex_upload_buffer,
                            _vertex_buffer))
            EngineAssert(false);

    } else {
        _dynamic_buffer = std::make_unique<UploadBuffer>(device, numVertex, static_cast<UINT>(vertexSize), false);
        for (UINT i = 0; i < numVertex; ++i)
            _dynamic_buffer->Copy(i, vertices);
    }
    UploadResource(device, cmd_list, indices, _index_buffer_byte_size, _index_upload_buffer, _index_buffer);

    _vertex_byte_stride = vertexSize;

};

bool VertexBuffer::UpdateVertexData(DeviceCom *device, void *vertices, UINT numVertex, UINT vertexSize) {
    if (!_isDynamicBuffer) {
        GRAPHICS_LOG_ERROR("Cannot update vertices buffer. The buffer was static.");
        return false;
    }

    if (numVertex * vertexSize != _vertex_buffer_byte_size) {
        _dynamic_buffer.release();
        _dynamic_buffer = std::make_unique<UploadBuffer>(device, numVertex, static_cast<UINT>(vertexSize), false);
    }

    EngineAssert(_dynamic_buffer != nullptr);
    for (UINT i = 0; i < numVertex; ++i)
        _dynamic_buffer->Copy(i, vertices);

    return true;
}

D3D12_VERTEX_BUFFER_VIEW VertexBuffer::VertexBufferView() const {
    D3D12_VERTEX_BUFFER_VIEW view;
    if (_isDynamicBuffer)
        view.BufferLocation = _dynamic_buffer->Resource()->GetGPUVirtualAddress();
    else
        view.BufferLocation = _vertex_buffer->GetGPUVirtualAddress();
    view.StrideInBytes = _vertex_byte_stride;
    view.SizeInBytes = _vertex_buffer_byte_size;
    return view;
}

D3D12_INDEX_BUFFER_VIEW VertexBuffer::IndexBufferView() const {
    D3D12_INDEX_BUFFER_VIEW view;
    view.BufferLocation = _index_buffer->GetGPUVirtualAddress();
    view.SizeInBytes = _index_buffer_byte_size;
    view.Format = _index_format;
    return view;
}
_END_ENGINE