//
// Created by khhan on 2021-09-02.
//

#include "src/asset_manager.h"
#include "src/global.h"
#include "src/k3d_app.h"

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
    auto textureMap = resourceMap->TextureMap.get();

    auto allTextures = textureMap->GetTextureList();
    auto allMeshes = meshMap->GetMeshList();
    auto allMaterials = matMap->GetMaterialList();

    json["Textures"] = allTextures;
    json["Meshes"] = allMeshes;

    {
        for (auto &s : allMaterials) {
            auto matName = s;
            auto matDesc = matMap->GetMaterialDesc(matName);
            auto matType = matDesc->MaterialType;
            auto diffuseTexturePath = matDesc->BaseColorTexturePath;
            auto refractIndex = matDesc->RefractIndex;
            auto specularPower = matDesc->SpecularPower;
            auto emittedColor = matDesc->EmissiveColor;
            auto baseColor = matDesc->BaseColor;
            auto roughness = matDesc->Roughness;
            auto metallic = matDesc->Metallic;
            auto useTexture = matDesc->UseBaseColorTexture;

            json["Materials"].push_back({
                                                    {"MaterialName",       matName},
                                                    {"MaterialType",       matType},
                                                    {"DiffuseTexturePath", diffuseTexturePath},
                                                    {"RefractIndex",       refractIndex},
                                                    {"Metallic",          metallic},
                                                    {"Roughness", roughness},
                                                    {"SpecularPower", specularPower},
                                                    {"EmissiveColor", {emittedColor.x, emittedColor.y, emittedColor.z}},
                                                    {"BaseColor",       {baseColor.x, baseColor.y, baseColor.z}},
                                                    {"UseTexture",         useTexture}
                                            });
        }
    }

    {
        for (auto &s : renObjStrV) {
            auto renObj = screen_ptr->GetRenderObject(s);

            auto matName = renObj->MaterialName;
            auto meshResource = meshMap->GetResources(renObj->MeshID);
            auto meshPath = meshResource->MeshFile.string();
            auto subMesh = renObj->SubmeshID;


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
                                                    {"WorldMatrix",        nlohmann::json(worldMatrixElem)},
                                                    {"MaterialName",       matName},
                                            });
        }
    }

    {
        for (auto &s : lightStrV) {
            auto light = screen_ptr->GetLight(s);

            int light_type = static_cast<int>(light->LightType);
            float4x4 worldMatrix;
            DirectX::XMStoreFloat4x4(&worldMatrix, light->GetWorldMatrix());
            std::vector<float> worldMatrixElem;
            for (int i = 0; i < 4 * 4; i++) {
                worldMatrixElem.push_back(reinterpret_cast<float *>(&worldMatrix)[i]);
            }
            float intensity = light->I;

            json["Lights"].push_back({
                                             {"Name",        s},
                                             {"WorldMatrix", nlohmann::json(worldMatrixElem)},
                                             {"LightType", light_type},
                                             {"Intensity", intensity}
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
                {"Target",   {camera->Target.x,   camera->Target.y,   camera->Target.z}},
                {"Up", {camera->CameraUp.x, camera->CameraUp.y, camera->CameraUp.z}}
        };
    }

    auto height = viewportWindow->GetHeight();
    auto width = viewportWindow->GetWidth();
    auto depth = viewportWindow->GetTraceDepth();
    auto env_map_path_opt = viewportWindow->GetRenderScreen()->EnvTextureKey;
    std::string env_map;
    if (env_map_path_opt.has_value()) {
        env_map = env_map_path_opt.value();
    }else {
        env_map = "";
    }

    {
        json["Options"] = {
                {"Height", height},
                {"Width",  width},
                {"EnvMap", env_map},
                {"VSync", viewportWindow->VsyncEnabled()},
                {"Depth", depth}
        };
    }

    auto dumped = json.dump();
    std::ofstream writer(savePath.string());
    if (writer.is_open()) {
        writer << std::setw(4) << dumped << std::endl;
        return true;
    }

    return false;
}

bool
AssetManager::Load(std::filesystem::path loadFile, ViewportWindow *viewportWindow,
                   _ENGINE::Renderer::ResourceMap *resourceMap) {

    viewportWindow->SelectedObject = nullptr;
    auto screen_ptr = viewportWindow->GetRenderScreen();

    auto matMap = resourceMap->MaterialMap.get();
    auto meshMap = resourceMap->MeshMap.get();
    auto textureMap = resourceMap->TextureMap.get();

    //Clear Screen
    auto renObjStrV = screen_ptr->GetRenderObjectList();
    auto lightStrV = screen_ptr->GetLightList();
    for (auto& s : renObjStrV) {
        viewportWindow->GetRenderScreen()->UnRegisterRenderObject(s);
    }

    for (auto& s : lightStrV) {
        viewportWindow->GetRenderScreen()->RemoveLight(s);
    }

    matMap->Clear();
    meshMap->Clear();
    textureMap->Clear();

    std::ifstream inputs(loadFile.string());
    if (!inputs.is_open()) {
        return false;
    }

    nlohmann::json json;
    inputs >> json;

    auto camera_far = json["Camera"]["Far"].get<float>();
    auto camera_fov = json["Camera"]["Fov"].get<float>();
    auto camera_near = json["Camera"]["Near"].get<float>();
    auto camera_ratio = json["Camera"]["Ratio"].get<float>();
    auto camera_target = json["Camera"]["Target"].get<std::vector<float>>();
    auto camera_position = json["Camera"]["Position"].get<std::vector<float>>();
    auto camera_up = json["Camera"]["Up"].get<std::vector<float>>();

    auto option_height = json["Options"]["Height"].get<UINT>();
    auto option_width= json["Options"]["Width"].get<UINT>();
    auto env_map = json["Options"]["EnvMap"].get<std::string>();
    auto vsync = json["Options"]["VSync"].get<bool>();
    auto depth = json["Options"]["Depth"].get<UINT>();

    if (env_map.empty()) {
        viewportWindow->GetRenderScreen()->EnvTextureKey = std::nullopt;
    }else {
        viewportWindow->GetRenderScreen()->EnvTextureKey = env_map;
    }
    viewportWindow->SetVSync(vsync);
    viewportWindow->ResetCameraAngle();
    viewportWindow->SetResolution(option_width, option_height);
    viewportWindow->SetTraceDepth(depth);

    auto camera = viewportWindow->GetCamera();
    camera->Far = camera_far;
    camera->Fov = camera_fov;
    camera->Near = camera_near;
    camera->Ratio = camera_ratio;

    DirectX::XMFLOAT3 target(camera_target.data());
    DirectX::XMFLOAT3 position(camera_position.data());
    DirectX::XMFLOAT3 up(camera_up.data());

    camera->SetPosition(DirectX::XMLoadFloat3(&position));
    camera->LookAt(DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));

    std::vector<_ENGINE::MaterialDesc> mat_descs;
    std::vector<_ENGINE::RenderObject> renobjs;
    std::vector<_ENGINE::Light> lights;

    std::set<std::filesystem::path> textures;
    std::set<std::filesystem::path> meshes;

    auto system_texture_path = global::AssetPath / "textures" / "default_texture.png";
    auto system_mesh_path = global::AssetPath / "meshes" / "sphere.obj";

    auto allTextures = json["Textures"].get<std::vector<std::string>>();
    auto allMeshes = json["Meshes"].get<std::vector<std::string>>();

    for (auto& t : allTextures) {
        textures.insert(t);
    }

    for (auto& m : allMeshes) {
        meshes.insert(m);
    }

    for (UINT i = 0; i < json["Materials"].size(); i++) {
        auto& material = json["Materials"][i];
        auto diffuse_texture_path = material["DiffuseTexturePath"].get<std::string>();
        auto material_type = material["MaterialType"].get<int>();
        auto refract_idx = material["RefractIndex"].get<int>();
        auto metallic = material["Metallic"].get<float>();
        auto roughness = material["Roughness"].get<float>();
        auto specular_power = material["SpecularPower"].get<float>();
        auto mat_name = material["MaterialName"].get<std::string>();
        auto emissive_color = material["EmissiveColor"].get<std::vector<float>>();
        auto base_color = material["BaseColor"].get<std::vector<float>>();
        auto use_texture = material["UseTexture"].get<bool>();

        _ENGINE::MaterialDesc mat_desc;
        mat_desc.SpecularPower = specular_power;
        mat_desc.RefractIndex = refract_idx;
        mat_desc.Metallic = metallic;
        mat_desc.Roughness = roughness;
        mat_desc.BaseColorTexturePath = diffuse_texture_path;
        mat_desc.MaterialType = static_cast<_ENGINE::ShaderType::MaterialType>(material_type);
        mat_desc.EmissiveColor = DirectX::XMFLOAT3(emissive_color.data());
        mat_desc.BaseColor = DirectX::XMFLOAT3(base_color.data());
        mat_desc.UseBaseColorTexture = use_texture;

        if (!matMap->Contains(mat_name))
        {
            matMap->AddMaterial(mat_name, mat_desc);
        }
    }

    for (UINT i = 0; i < json["RenderObjects"].size(); i++) {
        auto& renobj_json = json["RenderObjects"][i];

        auto mesh_path = renobj_json["MeshPath"].get<std::string>();
        auto submesh = renobj_json["SubMesh"].get<int>();
        auto name = renobj_json["Name"].get<std::string>();
        auto mat_name = renobj_json["MaterialName"].get<std::string>();
        auto world_mat = renobj_json["WorldMatrix"].get<std::vector<float>>();

        screen_ptr->AddRenderObject(name, mat_name, mesh_path, submesh);

        DirectX::XMFLOAT4X4 fmat(world_mat.data());
        auto obj = screen_ptr->GetRenderObject(name);
        if (obj != nullptr) {
            obj->SetTransform(DirectX::XMLoadFloat4x4(&fmat));
        }
    }

    meshMap->AsyncLoad(std::vector<std::filesystem::path>(meshes.begin(), meshes.end()));
    textureMap->AsyncLoad(std::vector<std::filesystem::path>(textures.begin(), textures.end()));

    for (UINT i = 0; i < json["Lights"].size(); i++) {
        auto& light_json = json["Lights"][i];
        auto name = light_json["Name"].get<std::string>();
        auto type = light_json["LightType"].get<int>();
        auto world_mat = light_json["WorldMatrix"].get<std::vector<float>>();
        auto intensity = light_json["Intensity"].get<float>();

        DirectX::XMFLOAT4X4 fmat(world_mat.data());
        screen_ptr->AddLight(name, static_cast<_ENGINE::ShaderType::LightType>(type));
        auto light = screen_ptr->GetLight(name);
        if (light != nullptr) {
            light->SetTransform(DirectX::XMLoadFloat4x4(&fmat));
            light->I = intensity;
        }
    }

    {
        //Add Default Material
        if (!matMap->Contains(K3DApp::DefaultMaterialName))
        {
            _ENGINE::MaterialDesc mat_desc {
                    .BaseColorTexturePath = system_texture_path.string(),
                    .MaterialType = _ENGINE::ShaderType::CookTorrance,
                    .RefractIndex = 1.5f,
                    .SpecularPower = 0.0f,
                    .Roughness = 0.0f,
                    .Metallic = 0.0f,
                    .EmissiveColor = float3(0.0f, 0.0f, 0.0f),
                    .BaseColor = float3(1.0f, 1.0f, 1.0f),
                    .UseBaseColorTexture = true,
            };
            matMap->AddMaterial(K3DApp::DefaultMaterialName, mat_desc);
        }
    }
    return true;
}
_END_KONAI3D