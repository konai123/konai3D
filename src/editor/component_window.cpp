//
// Created by korona on 2021-07-30.
//

#include "src/editor/component_window.h"

#include <utility>

#include "src/macros.h"
#include "src/engine/graphics/vertex.h"
#include "src/math.h"
#include "src/k3d_app.h"

_START_KONAI3D
ComponentWindow::ComponentWindow(
        std::shared_ptr<ViewportWindow> viewportWindow,
        std::shared_ptr<_ENGINE::Renderer::ResourceMap> resourceMap
)
:
IMGUIWindow("Components"),
_viewport_window(std::move(viewportWindow)),
_window_flags(ImGuiWindowFlags_MenuBar),
_render_resource_map(std::move(resourceMap)),
_mesh_file_dialog(ImGuiFileBrowserFlags_MultipleSelection)
{
    _mesh_file_dialog.SetTitle("Load meshes");
    _mesh_file_dialog.SetTypeFilters({".fbx", ".obj", ".ply"});
}

bool ComponentWindow::AddComponent(std::string name) {
    auto newRenderObj = _ENGINE::RenderObject::AllocRenderObject();
    newRenderObj->MaterialName = K3DApp::DefaultMaterialName;
    newRenderObj->MeshID = K3DApp::DefaultMeshName;
    newRenderObj->SubmeshID = 0;

    if (!_viewport_window->GetRenderScreen()->AddRenderObject(name, newRenderObj)) {
        return false;
    }
    return true;
}

bool ComponentWindow::DeleteComponent(std::string name) {
    if (_viewport_window->SelectedObject == _viewport_window->GetRenderScreen()->GetRenderObject(name))
        _viewport_window->SelectedObject = nullptr;

    if (!_viewport_window->GetRenderScreen()->UnRegisterRenderObject(name)) {
        APP_LOG_ERROR("Failed to delete render object '{}'", name);
        return false;
    }
    return true;
}

bool ComponentWindow::AddLight(std::string name) {
    _ENGINE::Light light;
    light.LightType = engine::ShaderType::LightType_Point;
    light.Position = float3(0.0f, 0.0f, 0.0f);

    if (!_viewport_window->GetRenderScreen()->AddLight(name, light)) {
        return false;
    }
    return true;
}

bool ComponentWindow::DeleteLight(std::string name) {
    if (_viewport_window->SelectedObject == _viewport_window->GetRenderScreen()->GetLight(name))
        _viewport_window->SelectedObject = nullptr;

    if (!_viewport_window->GetRenderScreen()->RemoveLight(name)) {
        return false;
    }
    return false;
};

void ComponentWindow::OnUpdate(float delta) {
    std::string name = GetWindowName();
    if (!ImGui::Begin(name.c_str(), &_open, _window_flags)) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginPopup("AddComponent")) {
        static char buf[20] = {0,};
        ImGui::InputText("Component Name", buf, 20);
        if (ImGui::Button("Ok")) {
            if (!AddComponent(buf)) {
                APP_LOG_ERROR("Failed to add '{}' component", buf);
            } else {
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("AddLight")) {
        static char buf[20] = {0,};
        ImGui::InputText("Light Name", buf, 20);
        if (ImGui::Button("Ok")) {
            if (!AddLight(buf)) {
                APP_LOG_ERROR("Failed to add '{}' Light", buf);
            } else {
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }

    if (ImGui::Button("Add Component")) {
        ImGui::OpenPopup("AddComponent");
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Light")) {
        ImGui::OpenPopup("AddLight");
    }

    auto names = _viewport_window->GetRenderScreen()->GetRenderObjectList();
    std::vector<const char*> c_names;
    for (auto& name : names) {
        c_names.push_back(name.data());
    }

    static int curr = -1;
    static int curr_light = -1;

    if (ImGui::ListBox("Components", &curr, c_names.data(), names.size())) {
        curr_light = -1;
    }
    ImGui::Separator();

    if (curr != -1 ) {
        auto name = names[curr];
        auto cmp =  _viewport_window->GetRenderScreen()->GetRenderObject(name);

        _viewport_window->SelectedObject = cmp;

        float4x4 world;
        DirectX::XMStoreFloat4x4(&world, cmp->WorldMatrix);
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        ImGuizmo::DecomposeMatrixToComponents(
                reinterpret_cast<float*>(&world),
                matrixTranslation,
                matrixRotation,
                matrixScale
                );
        if (ImGui::InputFloat3("Tr", matrixTranslation)) _viewport_window->GetRenderScreen()->Updated = true;
        if (ImGui::InputFloat3("Rt", matrixRotation)) _viewport_window->GetRenderScreen()->Updated = true;
        if (ImGui::InputFloat3("Sc", matrixScale)) _viewport_window->GetRenderScreen()->Updated = true;
        ImGuizmo::RecomposeMatrixFromComponents(
                matrixTranslation,
                matrixRotation,
                matrixScale,
                reinterpret_cast<float*>(&world)
        );

        cmp->SetTransform(DirectX::XMLoadFloat4x4(&world));

        ImGui::Text("Material: ");
        ImGui::SameLine();
        std::string mat_name = cmp->MaterialName;
        if (ImGui::Button(mat_name.data())) {
            ImGui::OpenPopup("Material");
        }

        auto mat_desc = _render_resource_map->MaterialMap->GetMaterialDesc(mat_name);

        ImGui::Text("Mesh: ");
        ImGui::SameLine();

        if (ImGui::BeginPopup("Meshes")) {
            auto names = _render_resource_map->MeshMap->GetMeshList();
            if (ImGui::Button("From Files...")) {
                _mesh_file_dialog.Open();
            }

            for (auto &name : names) {
                ImGui::PushID(name.data());
                if (ImGui::Button(name.data())) {
                    cmp->UpdateMesh(name, 0);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopID();
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("Material")) {
            auto names = _render_resource_map->MaterialMap->GetMaterialList();
            for (auto &name : names) {
                ImGui::PushID(name.data());
                if (ImGui::Button(name.data())) {
                    cmp->UpdateMaterial(name);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopID();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Button(cmp->MeshID.c_str())) {
            ImGui::OpenPopup("Meshes");
        }

        if (ImGui::Button("Delete")) {
            DeleteComponent(name);
            curr = -1;
        }
    }


    auto light_names = _viewport_window->GetRenderScreen()->GetLightList();
    std::vector<const char*> c_light_names;
    for (auto& name : light_names) {
        c_light_names.push_back(name.data());
    }

    ImGui::Separator();
    if (ImGui::ListBox("Lights", &curr_light, c_light_names.data(), c_light_names.size())) {
        curr = -1;
    }
    ImGui::Separator();

    if (curr_light != -1 ) {
        auto name = light_names[curr_light];
        auto light =  _viewport_window->GetRenderScreen()->GetLight(name);

        _viewport_window->SelectedObject = light;

        float4x4 world;
        DirectX::XMStoreFloat4x4(&world, light->GetWorldMatrix());
        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        ImGuizmo::DecomposeMatrixToComponents(
                reinterpret_cast<float*>(&world),
                matrixTranslation,
                matrixRotation,
                matrixScale
        );
        if (ImGui::InputFloat3("Tr", matrixTranslation))  _viewport_window->GetRenderScreen()->Updated = true;
        if (ImGui::InputFloat3("Rt", matrixRotation))  _viewport_window->GetRenderScreen()->Updated = true;
        if (ImGui::InputFloat3("Sc", matrixScale))  _viewport_window->GetRenderScreen()->Updated = true;
        ImGuizmo::RecomposeMatrixFromComponents(
                matrixTranslation,
                matrixRotation,
                matrixScale,
                reinterpret_cast<float*>(&world)
        );

        light->SetTransform(DirectX::XMLoadFloat4x4(&world));

        if (ImGui::Button("Delete")) {
            DeleteLight(name);
            curr_light = -1;
        }
    }

    ImGui::End();

    /*file browsers*/
    _mesh_file_dialog.Display();
    if (_mesh_file_dialog.HasSelected()) {
        auto selected = _mesh_file_dialog.GetMultiSelected();
        std::vector<std::string> v;
        for (UINT i  = 0; i < selected.size(); i++) {
            APP_LOG_INFO("Load Model: {}", selected[i].string());
        }
        _render_resource_map->MeshMap->AsyncLoad(selected);
        _mesh_file_dialog.ClearSelected();
    }
}

void ComponentWindow::OnDestroy() {
}

_END_KONAI3D