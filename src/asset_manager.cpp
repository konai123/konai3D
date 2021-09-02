//
// Created by khhan on 2021-09-02.
//

#include "src/asset_manager.h"

_START_KONAI3D
AssetManager &AssetManager::Instance() {
    static AssetManager inst;
    return inst;
}

AssetManager::AssetManager() {}

bool
AssetManager::Save(std::filesystem::path savePath, ViewportWindow *viewportWindow,
                   _ENGINE::Renderer::ResourceMap *resourceMap) {
    nlohmann::json json;
    auto screen_ptr = viewportWindow->GetRenderScreen();
    auto renObjStrV = screen_ptr->GetRenderObjectList();
    auto lightStrV = screen_ptr->GetLightList();

    auto matMap = resourceMap->MaterialMap.get();
    auto meshMap = resourceMap->MeshMap.get();

    {
        for (auto &s : renObjStrV) {
            auto renObj = screen_ptr->GetRenderObject(s);

            auto matName = renObj->MaterialName;
            auto matDesc = matMap->GetMaterialDesc(matName);
            auto meshResource = meshMap->GetResources(renObj->MeshID);

            auto meshPath = meshResource->MeshFile.string();
            auto subMesh = renObj->SubmeshID;
            auto matType = matDesc->MaterialType;
            auto diffuseTexturePath = matDesc->DiffuseTexturePath;
            auto refractIndex = matDesc->RefractIndex;
            auto fuzz = matDesc->Fuzz;

            float4x4 worldMatrix;
            DirectX::XMStoreFloat4x4(&worldMatrix, renObj->GetWorldMatrix());

            std::vector<float> worldMatrixElem;
            for (int i = 0; i < 4 * 4; i++) {
                worldMatrixElem.push_back(reinterpret_cast<float *>(&worldMatrix)[i]);
            }

            json["RenderObjects"].push_back({
                                                    {"Name",               s},
                                                    {"MeshPath",           meshPath},
                                                    {"SubMesh",            subMesh},
                                                    {"MaterialType",       matType},
                                                    {"DiffuseTexturePath", diffuseTexturePath},
                                                    {"RefractIndex",       refractIndex},
                                                    {"FuzzValue",          fuzz},
                                                    {"WorldMatrix",        nlohmann::json(worldMatrixElem)}
                                            });
        }
    }

    {
        for (auto &s : lightStrV) {
            auto light = screen_ptr->GetLight(s);

            float4x4 worldMatrix;
            DirectX::XMStoreFloat4x4(&worldMatrix, light->GetWorldMatrix());
            std::vector<float> worldMatrixElem;
            for (int i = 0; i < 4 * 4; i++) {
                worldMatrixElem.push_back(reinterpret_cast<float *>(&worldMatrix)[i]);
            }

            json["Lights"].push_back({
                                             {"Name",        s},
                                             {"WorldMatrix", nlohmann::json(worldMatrixElem)}
                                     });
        }
    }

    auto camera = viewportWindow->GetCamera();
    {
        json["Camera"] = {
                {"Fov",      camera->Fov},
                {"Ratio",    camera->Ratio},
                {"Near",     camera->Near},
                {"Far",      camera->Far},
                {"Position", {camera->Position.x, camera->Position.y, camera->Position.z}},
                {"Target",   {camera->Target.x,   camera->Target.y,   camera->Target.z}}
        };
    }

    auto height = viewportWindow->GetHeight();
    auto width = viewportWindow->GetWidth();
    {
        json["Options"] = {
                {"Height", height},
                {"Width",  width}
        };
    }

    auto dumped = json.dump();
    std::ofstream writer(savePath.string() + ".json");
    if (writer.is_open()) {
        writer << std::setw(4) << dumped << std::endl;
        return true;
    }

    return false;
}

bool
AssetManager::Load(std::filesystem::path loadFile, ViewportWindow *viewportWindow,
                   _ENGINE::Renderer::ResourceMap *resourceMap) {
    return false;
}
_END_KONAI3D