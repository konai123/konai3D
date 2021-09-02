//
// Created by korona on 2021-07-19.
//

#pragma once

#include "src/engine/core/rwlock.hpp"

#include "src/engine/graphics/mesh_loader.h"
#include "src/engine/graphics/macros.h"
#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/blas.h"

_START_ENGINE
struct MeshResources {
    struct MeshResource {
        UINT VertexBufferByteSize;
        UINT IndexBufferByteSize;
        UINT IndexCount;
        UINT VertexCount;

        UINT StartIndexLocation;
        UINT BaseVertexLocation;
        Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuffer;
        std::string Name;
        std::unique_ptr<BLAS> Blas;
    };

    std::vector<std::unique_ptr<MeshResource>> Meshes;
    std::filesystem::path MeshFile;
};

class MeshMap {
public:
    explicit MeshMap(std::shared_ptr<DeviceCom> deviceCom);
    virtual ~MeshMap() = default;

public:
    void AsyncLoad(std::vector<std::filesystem::path> paths);
    void UpdateFromMeshLoader(DirectX::ResourceUploadBatch* uploader, ID3D12GraphicsCommandList6* cmd_list);
    bool AddMeshes(MeshFile&& meshes, DirectX::ResourceUploadBatch* uploader, ID3D12GraphicsCommandList6* cmd_list);
    UINT UploadQueueSize();
    std::vector<std::string> GetMeshList();

public:
    bool Contains(std::string name);
    MeshResources* GetResources(std::string name);

private:
    std::shared_ptr<DeviceCom> _device;
    std::unordered_map<std::string, std::unique_ptr<MeshResources>> _map;
    MeshLoader _mesh_loader;
    RWLock _rw_lock;
    UINT64 _nonamed_index;
};

_END_ENGINE
