//
// Created by korona on 2021-07-19.
//

#pragma once

#include "src/engine/core/rwlock.hpp"

#include "src/engine/graphics/mesh_loader.h"
#include "src/engine/graphics/macros.h"
#include "src/engine/graphics/device_com.h"

_START_ENGINE
struct MeshResources {
    int _vb_byte_size;
    int _ib_byte_size;
    Microsoft::WRL::ComPtr<ID3D12Resource> _vertex_buffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> _index_buffer;
};

class MeshMap {
public:
    explicit MeshMap(std::shared_ptr<DeviceCom> deviceCom);
    virtual ~MeshMap() = default;

public:
    void AsyncLoad(std::vector<std::filesystem::path> paths);
    void UpdateFromMeshLoader(DirectX::ResourceUploadBatch* uploader);
    bool AddMeshes(std::vector<Mesh>&& meshes, DirectX::ResourceUploadBatch* uploader);
    std::vector<std::string> GetMeshList();

public:
    bool Contains(std::string name);
    MeshResources* GetResource(std::string name);

private:
    std::shared_ptr<DeviceCom> _device;
    std::unordered_map<std::string, std::unique_ptr<MeshResources>> _map;
    MeshLoader _mesh_loader;
    RWLock _rw_lock;
};

_END_ENGINE
