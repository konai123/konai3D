//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/mesh_map.h"

_START_ENGINE
MeshMap::MeshMap(std::shared_ptr<DeviceCom> deviceCom)
:
_device(deviceCom)
{
}

void MeshMap::AsyncLoad(std::vector<std::filesystem::path> paths) {
    _mesh_loader.AsyncLoad(paths);
}

void MeshMap::UpdateFromMeshLoader(DirectX::ResourceUploadBatch* uploader) {
    auto v = _mesh_loader.Get();
    for (auto& meshFile : v) {
        AddMeshes(std::move(meshFile.mesh), uploader);
    }
}

bool MeshMap::AddMeshes(std::vector<Mesh>&& meshes, DirectX::ResourceUploadBatch* uploader) {
    for (auto &mesh : meshes) {

        std::unique_ptr<MeshResources> drawInfo = std::make_unique<MeshResources>();

        auto name = mesh.name;
        if (_map.contains(name)) {
            GRAPHICS_LOG_WARNING("'{}' Already has been registered.", name);
            continue;
        }

        {
            UINT vertex_bytes_size = mesh.vertex_bytes_size;
            CD3DX12_RESOURCE_DESC vb_desc = CD3DX12_RESOURCE_DESC::Buffer(vertex_bytes_size);
            auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            auto vertexBuffer = _device->CreateResource(&properties, &vb_desc, nullptr, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON);

            if (vertexBuffer == nullptr) {
                GRAPHICS_LOG_ERROR("Cannot create vertex buffer");
                return false;
            }

            D3D12_SUBRESOURCE_DATA subresource = {};
            subresource.pData = mesh.vertices.data();
            subresource.RowPitch = vertex_bytes_size;
            subresource.SlicePitch = vertex_bytes_size;

            uploader->Upload(vertexBuffer.Get(), 0, &subresource, 1);
            drawInfo->_vertex_buffer = vertexBuffer;
            drawInfo->_vb_byte_size = vertex_bytes_size;
            vertexBuffer.Reset();
        }

        {
            UINT index_bytes_size = mesh.index_bytes_size;
            CD3DX12_RESOURCE_DESC ib_desc = CD3DX12_RESOURCE_DESC::Buffer(index_bytes_size);
            auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            auto indexBuffer = _device->CreateResource(&properties, &ib_desc, nullptr, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON);

            if (indexBuffer == nullptr) {
                GRAPHICS_LOG_ERROR("Cannot create vertex buffer");
                return false;
            }

            D3D12_SUBRESOURCE_DATA subresource = {};
            subresource.pData = mesh.indices.data();
            subresource.RowPitch = index_bytes_size;
            subresource.SlicePitch = index_bytes_size;

            uploader->Upload(indexBuffer.Get(), 0, &subresource, 1);
            drawInfo->_index_buffer = indexBuffer;
            drawInfo->_ib_byte_size = index_bytes_size;
            indexBuffer.Reset();
        }

        LocalWriteLock lock(_rw_lock);
        _map[name] = std::move(drawInfo);
    }
    return true;
}

std::vector<std::string> MeshMap::GetMeshList() {
    LocalReadLock lock(_rw_lock);
    std::vector<std::string> names;
    for (auto &p : _map) {
        names.push_back(p.first);
    }
    return names;
}

bool MeshMap::Contains(std::string name) {
    LocalReadLock lock(_rw_lock);
    return _map.contains(name);
}

MeshResources *MeshMap::GetResource(std::string name) {
    if (!Contains(name)) {
        GRAPHICS_LOG_ERROR("Cannot find mesh resource: {}", name);
        return nullptr;
    }

    LocalReadLock lock(_rw_lock);
    return _map[name].get();
}

_END_ENGINE