//
// Created by korona on 2021-07-31.
//

#pragma once

#include "src/engine/core/async_loader.h"
#include "src/engine/graphics/vertex.h"

_START_ENGINE
struct Mesh {
    std::string Name;
    std::vector<_ENGINE::Vertex> Vertices;
    std::vector<UINT32> Indices;
    UINT VertexBytesSize;
    UINT IndexBytesSize;
};

struct MeshFile {
public:
    MeshFile() {
        Clear();
    }

public:
    bool Valid() {
        if (Mesh.empty()) return false;
        return true;
    }

    void Clear() {
        Mesh.clear();
    }

public:
    std::vector<Mesh> Mesh;
    std::filesystem::path FilePath;
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
    std::mutex _importer_lock;
};

_END_ENGINE
