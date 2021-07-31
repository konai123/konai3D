//
// Created by korona on 2021-07-31.
//

#include "src/async_mesh_loader.h"
#include "src/macros.h"

_START_KONAI3D
namespace {
Assimp::Importer importer;
}

AsyncMeshLoader::AsyncMeshLoader() {}

void AsyncMeshLoader::Delegate(std::vector<std::string> paths) {
    for (UINT i = 0; i < paths.size(); i++) {
        Model model;
        std::string path = paths[i];
        model.Clear();
        model.name = path.substr(path.find_last_of("/\\") + 1);
        const aiScene *scene = importer.ReadFile(path, aiProcess_ConvertToLeftHanded |
                                                       aiProcess_JoinIdenticalVertices |
                                                       aiProcess_Triangulate |
                                                       aiProcess_SortByPType |
                                                       aiProcess_GenNormals |
                                                       aiProcess_GenUVCoords |
                                                       aiProcess_CalcTangentSpace |
                                                       aiProcess_OptimizeMeshes |
                                                       aiProcess_ValidateDataStructure);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            APP_LOG_ERROR(importer.GetErrorString());
        else {
            Process(model, scene->mRootNode, scene);
            Push(std::move(model));
        }
    }
}

void AsyncMeshLoader::Process(Model &model, aiNode *node, const aiScene *scene) {
    model.is_dynamic = false;

    for (UINT i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        To(model, mesh);
    }

    for (UINT i = 0; i < node->mNumChildren; i++) {
        Process(model, node->mChildren[i], scene);
    }
}

void AsyncMeshLoader::To(Model &model, aiMesh *_aiMesh) {
    Mesh mesh;

    for (UINT i = 0; i < _aiMesh->mNumVertices; i++) {
        Vertex vertex;
        auto aiVertex = _aiMesh->mVertices[i];
        auto aiNormal = _aiMesh->mNormals[i];
        auto aiTangent = _aiMesh->mTangents[i];
        auto aiTexCoord = _aiMesh->mTextureCoords[0][i];

        vertex._position = {aiVertex.x, aiVertex.y, aiVertex.z};
        vertex._normal = {aiNormal.x, aiNormal.y, aiNormal.z};
        vertex._tangent_u = {aiTangent.x, aiTangent.y, aiTangent.z};
        vertex._tex_coord = {aiTexCoord.x, aiTexCoord.y};
        mesh.vertices.push_back(vertex);
    }
    mesh.vertex_bytes_size = mesh.vertices.size() * sizeof(Vertex);

    for (UINT i = 0; i < _aiMesh->mNumFaces; i++) {
        auto face = _aiMesh->mFaces[i];
        for (UINT j = 0; j < face.mNumIndices; j++) {
            mesh.indices.push_back(face.mIndices[j]);
        }
    }
    mesh.index_bytes_size = mesh.indices.size() * sizeof(UINT);
    mesh.type = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    model.mesh.push_back(mesh);
}

_END_KONAI3D
