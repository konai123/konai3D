//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/mesh_map.h"
#include "src/engine/graphics/blas.h"

_START_ENGINE
MeshMap::MeshMap(std::shared_ptr<DeviceCom> deviceCom)
:
_device(deviceCom),
_nonamed_index(0)
{
}

void MeshMap::AsyncLoad(std::vector<std::filesystem::path> paths) {
    _mesh_loader.AsyncLoad(paths);
}

void MeshMap::UpdateFromMeshLoader(DirectX::ResourceUploadBatch* uploader, ID3D12GraphicsCommandList6* cmd_list) {
    auto v = _mesh_loader.Get();
    for (auto& meshFile : v) {
        AddMeshes(std::move(meshFile), uploader, cmd_list);
    }
}

bool MeshMap::AddMeshes(MeshFile&& meshes, DirectX::ResourceUploadBatch* uploader, ID3D12GraphicsCommandList6* cmd_list) {
    LocalWriteLock lock(_rw_lock);

    auto name = meshes.Name;
    if (_map.contains(name)) {
        GRAPHICS_LOG_WARNING("'{}' Already has been registered.", name);
        return false;
    }

    std::vector<Vertex> total_vertex;
    std::vector<UINT> total_index;

    for (UINT i = 0; i < meshes.Mesh.size(); i++) {
        total_vertex.insert(total_vertex.end(), meshes.Mesh[i].Vertices.begin(), meshes.Mesh[i].Vertices.end());
        total_index.insert(total_index.end(), meshes.Mesh[i].Indices.begin(), meshes.Mesh[i].Indices.end());
    }

    UINT total_vertex_bytes_size = total_vertex.size() * sizeof(Vertex);
    UINT total_index_bytes_size = total_index.size() * sizeof(UINT);

    CD3DX12_RESOURCE_DESC vb_desc = CD3DX12_RESOURCE_DESC::Buffer(total_vertex_bytes_size);
    auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto vertexBuffer = _device->CreateResource(&properties, &vb_desc, nullptr, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON);

    if (vertexBuffer == nullptr) {
        GRAPHICS_LOG_ERROR("Cannot create vertex buffer");
        return false;
    }

    D3D12_SUBRESOURCE_DATA subresource = {};
    subresource.pData = total_vertex.data();
    subresource.RowPitch = total_vertex_bytes_size;
    subresource.SlicePitch = total_vertex_bytes_size;

    uploader->Upload(vertexBuffer.Get(), 0, &subresource, 1);

    CD3DX12_RESOURCE_DESC ib_desc = CD3DX12_RESOURCE_DESC::Buffer(total_index_bytes_size);
    properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto indexBuffer = _device->CreateResource(&properties, &ib_desc, nullptr, D3D12_HEAP_FLAG_NONE, D3D12_RESOURCE_STATE_COMMON);

    if (indexBuffer == nullptr) {
        GRAPHICS_LOG_ERROR("Cannot create vertex buffer");
        return false;
    }

    subresource = {};
    subresource.pData = total_index.data();
    subresource.RowPitch = total_index_bytes_size;
    subresource.SlicePitch = total_index_bytes_size;

    uploader->Upload(indexBuffer.Get(), 0, &subresource, 1);

    auto mesh_resources = std::make_unique<MeshResources>();
    UINT ib_offset = 0;
    UINT vb_offset = 0;

    auto blas = std::make_unique<BLAS>();
    for (UINT i = 0; i < meshes.Mesh.size(); i++) {
        auto& mesh = meshes.Mesh[i];
        MeshResources::MeshResource info;

        info.Name = mesh.Name;
        info.IndexBuffer = indexBuffer;
        info.VertexBuffer = vertexBuffer;
        info.IndexBufferByteSize = mesh.IndexBytesSize;
        info.VertexBufferByteSize = mesh.VertexBytesSize;
        info.IndexCount = mesh.Indices.size();
        info.VertexCount = mesh.Vertices.size();
        info.StartIndexLocation = ib_offset;
        info.BaseVertexLocation = vb_offset;

        {
            //BLAS Build
            D3D12_RAYTRACING_GEOMETRY_DESC geo_desc;
            geo_desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
            geo_desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
            geo_desc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
            geo_desc.Triangles.VertexBuffer.StartAddress = info.VertexBuffer.Get()->GetGPUVirtualAddress() + vb_offset * sizeof(Vertex);
            geo_desc.Triangles.VertexFormat = Vertex::VertexFormat;
            geo_desc.Triangles.VertexCount = info.VertexCount;

            geo_desc.Triangles.IndexBuffer = info.IndexBuffer->GetGPUVirtualAddress() + ib_offset * sizeof(UINT);
            geo_desc.Triangles.IndexFormat = Vertex::IndexFormat;
            geo_desc.Triangles.IndexCount = info.IndexCount;
            geo_desc.Triangles.Transform3x4 = NULL;
            blas->AddGeometry(geo_desc);
        }

        ib_offset += mesh.Indices.size();
        vb_offset += mesh.Vertices.size();

        mesh_resources->Meshes.push_back(info);
    }

    if (!blas->Generate(_device.get(), cmd_list)) {
        GRAPHICS_LOG_ERROR("Cannot create bottom level acceleration structure");
        return false;
    }

    mesh_resources->Blas = std::move(blas);
    _map[name] = std::move(mesh_resources);
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

MeshResources* MeshMap::GetResources(std::string name) {
    if (!Contains(name)) {
        GRAPHICS_LOG_ERROR("Cannot find mesh resource: {}", name);
        return {};
    }

    LocalReadLock lock(_rw_lock);
    return _map[name].get();
}

_END_ENGINE