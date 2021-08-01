//
// Created by korona on 2021-06-20.
//

#ifndef KONAI3D_VERTEX_BUFFER_H
#define KONAI3D_VERTEX_BUFFER_H

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/upload_buffer.h"
#include "src/engine/graphics/macros.h"

_START_ENGINE

class VertexBuffer {
public:
    explicit VertexBuffer(DeviceCom *device,
                          ID3D12GraphicsCommandList *cmd_list,
                          void *vertices,
                          UINT numVertex,
                          UINT vertexSize,
                          UINT32 *indices,
                          UINT numIndex,
                          bool isDynamicBuffer);

    virtual ~VertexBuffer() = default;

public:
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const;
    D3D12_INDEX_BUFFER_VIEW IndexBufferView() const;
    bool UpdateVertexData(DeviceCom *device, void *vertex, UINT numVertex, UINT vertexSize);


private:
    inline static DXGI_FORMAT _index_format = DXGI_FORMAT_R32_UINT;
//Only use dynamic vertex buffer.
    std::unique_ptr<UploadBuffer> _dynamic_buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> _vertex_upload_buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> _index_upload_buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> _vertex_buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> _index_buffer;

    UINT _vertex_byte_stride;
    UINT _vertex_buffer_byte_size;
    UINT _index_buffer_byte_size;

    bool _isDynamicBuffer;
};
_END_ENGINE

#endif //KONAI3D_VERTEX_BUFFER_H
