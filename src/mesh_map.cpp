//
// Created by korona on 2021-07-30.
//

#include "src/mesh_map.h"

_START_KONAI3D
MeshMap::MeshMap(_ENGINE::Renderer *renderer) {
    std::vector<Mesh> meshes;
//General Meshes
    meshes.push_back(BoxMesh(1.0f, 1.0f, 1.0f));
    AddMeshes(_default_mesh, meshes, false, renderer);
}

void MeshMap::AyncLoad(std::vector<std::filesystem::path> paths) {
    _mesh_loader.Load(paths);
}

void MeshMap::UpdateFromMeshLoader(std::weak_ptr<_ENGINE::Renderer> renderer) {
    auto v = _mesh_loader.Get();
    for (auto& model : v) {
        auto renderer_shared = renderer.lock();
        if (renderer_shared == nullptr)
            break;
        AddMeshes(model.name, model.mesh, model.is_dynamic, renderer_shared.get());
    }
}

bool MeshMap::AddMeshes(std::string name, std::vector<Mesh> meshes, bool isDynamicMeshes, _ENGINE::Renderer *renderer) {
    std::vector<Vertex> vertices;
    std::vector<UINT32> indices;
    for (auto &mesh : meshes) {
        vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
        indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
    }

    auto vb = renderer->InstanceVertexBuffer(vertices.data(),
                                             static_cast<UINT>(vertices.size()),
                                             sizeof(Vertex),
                                             indices.data(),
                                             static_cast<UINT>(indices.size()),
                                             isDynamicMeshes);

    if (vb == nullptr) {
        APP_LOG_ERROR("Cannot instance vertex buffer");
        return false;
    }

    int current_index_location = 0;
    int current_vertex_location = 0;
    for (auto &mesh : meshes) {
        _ENGINE::LocalWriteLock lock(_rw_lock);
        if (_map.contains(name)) {
            APP_LOG_WARNING("'{}' Already has been registered.", name);
            continue;
        }
        std::shared_ptr<_ENGINE::DrawInfo> drawInfo = std::make_shared<_ENGINE::DrawInfo>();
        drawInfo->_index_count = static_cast<UINT>(mesh.indices.size());
        drawInfo->_start_index_location = current_index_location;
        drawInfo->_base_vertex_location = current_vertex_location;
        drawInfo->_type = mesh.type;
        drawInfo->_vertex_buffer = vb;
        _map[name] = drawInfo;
        current_index_location = mesh.index_bytes_size;
        current_vertex_location = mesh.vertex_bytes_size;
    }
    return true;
}

std::shared_ptr<_ENGINE::DrawInfo> MeshMap::GetDrawInfo(std::string name) {
    _ENGINE::LocalReadLock lock(_rw_lock);
    if (!_map.contains(name)) return nullptr;
    return _map[name];
}

std::vector<std::string> MeshMap::GetMeshList() {
    _ENGINE::LocalReadLock lock(_rw_lock);
    std::vector<std::string> names;
    for (auto &p : _map) {
        names.push_back(p.first);
    }
    return names;
}

_END_KONAI3D