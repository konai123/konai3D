//
// Created by korona on 2021-07-19.
//

#ifndef KONAI3D_MESH_MAP_H
#define KONAI3D_MESH_MAP_H

#include "src/engine/graphics/renderer.h"
#include "src/engine/core/rwlock.hpp"

#include "src/vertex.h"
#include "src/async_mesh_loader.h"
#include "src/macros.h"

_START_KONAI3D
//Demo code
namespace {
    Mesh BoxMesh(float width, float height, float depth) {
        Mesh mesh;
        auto& vs = mesh.vertices;
        float hw = width*0.5f;
        float hh = height*0.5f;
        float hd = depth*0.5f;

        _USING_ENGINE
        //Front Face        /* POSITION   /     NORMAL      /     TANGENT(U)   /   UV      /*/
        vs.push_back(Vertex(-hw, -hh, -hd, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
        vs.push_back(Vertex(-hw, +hh, -hd, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
        vs.push_back(Vertex(+hw, +hh, -hd, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f));
        vs.push_back(Vertex(+hw, -hh, -hd, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f));

        //Back Face
        vs.push_back(Vertex(-hw, -hh, +hd, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f));
        vs.push_back(Vertex(+hw, -hh, +hd, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
        vs.push_back(Vertex(+hw, +hh, +hd, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
        vs.push_back(Vertex(-hw, +hh, +hd, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f));

        //Top Face
        vs.push_back(Vertex(-hw, +hh, -hd, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
        vs.push_back(Vertex(-hw, +hh, +hd, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
        vs.push_back(Vertex(+hw, +hh, +hd, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f));
        vs.push_back(Vertex(+hw, +hh, -hd, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f));

        //Bottom Face
        vs.push_back(Vertex(-hw, -hh, -hd, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f));
        vs.push_back(Vertex(+hw, -hh, -hd, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
        vs.push_back(Vertex(+hw, -hh, +hd, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
        vs.push_back(Vertex(-hw, -hh, +hd, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f));

        //Left Face
        vs.push_back(Vertex(-hw, -hh, +hd, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f));
        vs.push_back(Vertex(-hw, +hh, +hd, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f));
        vs.push_back(Vertex(-hw, +hh, -hd, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f));
        vs.push_back(Vertex(-hw, -hh, -hd, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f));

        //Right Face
        vs.push_back(Vertex(+hw, -hh, -hd, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f));
        vs.push_back(Vertex(+hw, +hh, -hd, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));
        vs.push_back(Vertex(+hw, +hh, +hd, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f));
        vs.push_back(Vertex(+hw, -hh, +hd, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));

        UINT32 i[36];
        i[0] = 0; i[1] = 1; i[2] = 2;
        i[3] = 0; i[4] = 2; i[5] = 3;

        i[6] = 4; i[7]  = 5; i[8]  = 6;
        i[9] = 4; i[10] = 6; i[11] = 7;

        i[12] = 8; i[13] =  9; i[14] = 10;
        i[15] = 8; i[16] = 10; i[17] = 11;

        i[18] = 12; i[19] = 13; i[20] = 14;
        i[21] = 12; i[22] = 14; i[23] = 15;

        i[24] = 16; i[25] = 17; i[26] = 18;
        i[27] = 16; i[28] = 18; i[29] = 19;

        i[30] = 20; i[31] = 21; i[32] = 22;
        i[33] = 20; i[34] = 22; i[35] = 23;

        mesh.indices.assign(&i[0], &i[36]);
        mesh.type = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        mesh.index_bytes_size = 36 * sizeof(UINT32);
        mesh.vertex_bytes_size = static_cast<UINT>(vs.size() * sizeof(Vertex));
        return mesh;
    }
}

class MeshMap {
public:
    MeshMap(_ENGINE::Renderer* renderer);
    virtual ~MeshMap() = default;

public:
    void AyncLoad(std::vector<std::filesystem::path> paths);
    void UpdateFromMeshLoader(std::weak_ptr<_ENGINE::Renderer> renderer);
    bool AddMeshes(std::string name, std::vector<Mesh> meshes, bool isDynamicMeshes, _ENGINE::Renderer* renderer);
    std::shared_ptr<_ENGINE::DrawInfo> GetDrawInfo(std::string name);
    std::vector<std::string> GetMeshList();

public:
    inline static const std::string _default_mesh = "default_mesh";

private:
    std::unordered_map<std::string, std::shared_ptr<_ENGINE::DrawInfo>> _map;
    AsyncMeshLoader _mesh_loader;
    _ENGINE::RWLock _rw_lock;
};

_END_KONAI3D

#endif //KONAI3D_MESH_MAP_H
