//
// Created by korona on 2021-07-31.
//

#ifndef KONAI3D_ASYNC_MESH_LOADER_H
#define KONAI3D_ASYNC_MESH_LOADER_H

#include "src/engine/core/async_loader.h"
#include "src/vertex.h"

_START_KONAI3D
struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<UINT32> indices;
    UINT vertex_bytes_size;
    UINT index_bytes_size;
    D3D_PRIMITIVE_TOPOLOGY type;
};

struct Model {
public:
    Model() {
        Clear();
    }

public:
    bool Valid() {
        if (mesh.empty()) return false;
        return true;
    }

    void Clear() {
        name = "";
        mesh.clear();
    }

public:
    std::string name;
    std::vector<Mesh> mesh;
    bool is_dynamic;
};

class AsyncMeshLoader : public _ENGINE::AsyncLoader<Model>{
public:
    AsyncMeshLoader();
    virtual ~AsyncMeshLoader() = default;

public:
    virtual void Delegate(std::vector<std::string> paths) override;

private:
    void Process(Model& model, aiNode *node, const aiScene *scene);
    void To(Model& model, aiMesh* mesh);
};

_END_KONAI3D

#endif //KONAI3D_ASYNC_MESH_LOADER_H
