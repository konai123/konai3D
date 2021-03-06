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

std::vector<std::unique_ptr<MeshResources>> MeshMap::FetchMeshLoader(DirectX::ResourceUploadBatch* uploader, ID3D12GraphicsCommandList6* cmd_list) {
    std::vector<std::unique_ptr<MeshResources>> vec;
    auto v = _mesh_loader.Get();
    for (auto& meshFile : v) {
        auto ptr = MakeMesheResource(std::move(meshFile), uploader, cmd_list);
        if (ptr != nullptr) {
            vec.push_back(std::move(ptr));
        }
    }
    return std::move(vec);
}
bool MeshMap::AddMesh(std::string key, std::unique_ptr<MeshResources> value) {
    LocalWriteLock lock(_rw_lock);

    if (_map.contains(key)) {
        GRAPHICS_LOG_WARNING("'{}' Already has been registered.", key);
        return false;
    }

    _map[key] = std::move(value);
    return true;
}

std::unique_ptr<MeshResources> MeshMap::MakeMesheResource(MeshFile&& meshes, DirectX::ResourceUploadBatch* uploader, ID3D12GraphicsCommandList6* cmd_list) {
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
        return nullptr;
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
        return nullptr;
    }

    subresource = {};
    subresource.pData = total_index.data();
    subresource.RowPitch = total_index_bytes_size;
    subresource.SlicePitch = total_index_bytes_size;

    uploader->Upload(indexBuffer.Get(), 0, &subresource, 1);

    auto mesh_resources = std::make_unique<MeshResources>();
    mesh_resources->MeshFile = meshes.FilePath;
    UINT ib_offset = 0;
    UINT vb_offset = 0;

    for (UINT i = 0; i < meshes.Mesh.size(); i++) {
        auto& mesh = meshes.Mesh[i];
        auto info = std::make_unique<MeshResources::MeshResource>();

        info->Name = mesh.Name;
        info->IndexBuffer = indexBuffer;
        info->VertexBuffer = vertexBuffer;
        info->IndexBufferByteSize = mesh.IndexBytesSize;
        info->VertexBufferByteSize = mesh.VertexBytesSize;
        info->IndexCount = mesh.Indices.size();
        info->VertexCount = mesh.Vertices.size();
        info->StartIndexLocation = ib_offset;
        info->BaseVertexLocation = vb_offset;

        {
            //BLAS Build
            auto blas = std::make_unique<BLAS>();
            D3D12_RAYTRACING_GEOMETRY_DESC geo_desc;
            geo_desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
            geo_desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
            geo_desc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
            geo_desc.Triangles.VertexBuffer.StartAddress = info->VertexBuffer.Get()->GetGPUVirtualAddress() + vb_offset * sizeof(Vertex);
            geo_desc.Triangles.VertexFormat = Vertex::VertexFormat;
            geo_desc.Triangles.VertexCount = info->VertexCount;

            geo_desc.Triangles.IndexBuffer = info->IndexBuffer->GetGPUVirtualAddress() + ib_offset * sizeof(UINT);
            geo_desc.Triangles.IndexFormat = Vertex::IndexFormat;
            geo_desc.Triangles.IndexCount = info->IndexCount;
            geo_desc.Triangles.Transform3x4 = NULL;
            blas->AddGeometry(geo_desc);

            if (!blas->Generate(_device.get(), cmd_list)) {
                GRAPHICS_LOG_ERROR("Cannot create bottom level acceleration structure");
                return nullptr;
            }
            info->Blas = std::move(blas);
        }

        ib_offset += mesh.Indices.size();
        vb_offset += mesh.Vertices.size();

        mesh_resources->Meshes.push_back(std::move(info));
    }

    return std::move(mesh_resources);
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

UINT MeshMap::UploadQueueSize() {
    return _mesh_loader.Size();
}

void MeshMap::Clear() {
    LocalWriteLock lock(_rw_lock);
    _mesh_loader.Get();

    for (auto& item : _map) {
        auto resource = item.second.get();
        auto& Meshes = resource->Meshes;
        for (UINT i = 0; i < Meshes.size(); i++) {
            auto p = Meshes[i].get();
            if (p->VertexBuffer != nullptr)
                ResourceGarbageQueue::Instance().SubmitResource(p->VertexBuffer);
            if (p->IndexBuffer != nullptr)
                ResourceGarbageQueue::Instance().SubmitResource(p->IndexBuffer);
            if (p->Blas->ResultDataBuffer != nullptr)
                ResourceGarbageQueue::Instance().SubmitResource(p->Blas->ResultDataBuffer);
            if (p->Blas->ScratchBuffer != nullptr)
                ResourceGarbageQueue::Instance().SubmitResource(p->Blas->ScratchBuffer);
        }
    }
    _map.clear();
    _nonamed_index = 0;
}

MeshResources* MeshMap::GetResources(std::string name) {
    LocalReadLock lock(_rw_lock);
    if (!_map.contains(name)) {
        return {};
    }

    return _map[name].get();
}

_END_ENGINE