//
// Created by korona on 2021-07-31.
//

#pragma once

#include "src/engine/core/async_loader.h"
#include "src/engine/graphics/vertex.h"

_START_ENGINE
struct Mesh {
    std::string name;
    std::vector<_ENGINE::Vertex> vertices;
    std::vector<UINT32> indices;
    UINT vertex_bytes_size;
    UINT index_bytes_size;
};

struct MeshFile {
public:
    MeshFile() {
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
};

class MeshLoader : public _ENGINE::AsyncLoader<MeshFile> {
public:
    MeshLoader();
    virtual ~MeshLoader() = default;

public:
    virtual void Delegate(std::vector<std::filesystem::path> paths) override;
    std::optional<MeshFile> LoadMesh(std::filesystem::path path);

private:
    void Process(MeshFile& model, aiNode *node, const aiScene *scene);
    void To(MeshFile& model, aiMesh* mesh);
};

_END_ENGINE
