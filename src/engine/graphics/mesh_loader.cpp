//
// Created by korona on 2021-07-31.
//

#include "src/engine/graphics/mesh_loader.h"
#include "src/engine/graphics/macros.h"

_START_ENGINE
namespace {
Assimp::Importer importer;
}

MeshLoader::MeshLoader() {}

void MeshLoader::Delegate(std::vector<std::filesystem::path> paths) {
    for (UINT i = 0; i < paths.size(); i++) {
        auto mesh_file = LoadMesh(paths[i]);
        if (mesh_file) {
            Push(std::move(mesh_file.value()));
        }
    }
}

std::optional<MeshFile> MeshLoader::LoadMesh(std::filesystem::path path) {
    MeshFile mesh_file;
    std::string path_str = path.string();
    mesh_file.Clear();
    mesh_file.Name = path.filename().stem().string();
    const aiScene *scene = importer.ReadFile(path_str, aiProcess_ConvertToLeftHanded |
                                                   aiProcess_JoinIdenticalVertices |
                                                   aiProcess_Triangulate |
                                                   aiProcess_SortByPType |
                                                   aiProcess_GenNormals |
                                                   aiProcess_GenUVCoords |
                                                   aiProcess_CalcTangentSpace |
                                                   aiProcess_OptimizeMeshes |
                                                   aiProcess_ValidateDataStructure);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        GRAPHICS_LOG_ERROR(importer.GetErrorString());
        return std::nullopt;
    }
    else {
        Process(mesh_file, scene->mRootNode, scene);
    }
    return std::move(mesh_file);
}

void MeshLoader::Process(MeshFile &meshFile, aiNode *node, const aiScene *scene) {
    for (UINT i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        To(meshFile, mesh);
    }

    for (UINT i = 0; i < node->mNumChildren; i++) {
        Process(meshFile, node->mChildren[i], scene);
    }
}

void MeshLoader::To(MeshFile &meshFile, aiMesh *_aiMesh) {
    Mesh mesh;

    for (UINT i = 0; i < _aiMesh->mNumVertices; i++) {
        Vertex vertex;
        auto aiVertex = _aiMesh->mVertices[i];
        auto aiNormal = _aiMesh->mNormals[i];
        auto aiTangent = _aiMesh->mTangents[i];
        auto aiTexCoord = _aiMesh->mTextureCoords[0][i];

        vertex.Position = {aiVertex.x, aiVertex.y, aiVertex.z};
        vertex.Normal = {aiNormal.x, aiNormal.y, aiNormal.z};
        vertex.TangentU = {aiTangent.x, aiTangent.y, aiTangent.z};
        vertex.TexCoord = {aiTexCoord.x, aiTexCoord.y};
        mesh.Vertices.push_back(vertex);
    }
    mesh.VertexBytesSize = static_cast<UINT>(mesh.Vertices.size() * sizeof(Vertex));

    for (UINT i = 0; i < _aiMesh->mNumFaces; i++) {
        auto face = _aiMesh->mFaces[i];
        for (UINT j = 0; j < face.mNumIndices; j++) {
            mesh.Indices.push_back(face.mIndices[j]);
        }
    }
    mesh.IndexBytesSize = static_cast<UINT>(mesh.Indices.size() * sizeof(UINT));
    mesh.Name = _aiMesh->mName.C_Str();
    meshFile.Mesh.push_back(mesh);
}

_END_ENGINE
