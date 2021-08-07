//
// Created by korona on 2021-07-30.
//

#include "src/editor/component_window.h"

#include "src/macros.h"
#include "src/engine/graphics/vertex.h"
#include "src/math.h"
#include "src/k3d_app.h"

_START_KONAI3D
ComponentWindow::ComponentWindow(
        std::shared_ptr<_ENGINE::RenderScreen> screen,
        std::shared_ptr<_ENGINE::Renderer::ResourceMap> resourceMap
)
:
IMGUIWindow("Components"),
_screen(screen),
_window_flags(ImGuiWindowFlags_MenuBar),
_render_resource_map(resourceMap),
_mesh_file_dialog(ImGuiFileBrowserFlags_MultipleSelection)
{
    _mesh_file_dialog.SetTitle("AsyncLoad Mesh");
    _mesh_file_dialog.SetTypeFilters({".fbx", ".obj"});
}

bool ComponentWindow::AddComponent(std::string name) {
    if (_map.contains(name)) {
        APP_LOG_ERROR("Component {} already registered", name);
        return false;
    }

    std::unique_ptr<Component> new_cmp = std::make_unique<Component>(K3DApp::DefaultMaterialName, K3DApp::DefaultMeshName);
    if (!_screen->AddRenderObject(new_cmp->renderObject)) {
        return false;
    }

    _map[name] = std::move(new_cmp);
    return true;
}

bool ComponentWindow::DeleteComponent(std::string name) {
    if (!_map.contains(name)) {
        APP_LOG_ERROR("Component {} Unregistered", name);
        return false;
    }

    auto ptr = std::move(_map[name]);

    if (!_screen->UnRegisterRenderObject(ptr->renderObject->ObjectID)) {
        APP_LOG_ERROR("Failed to delete render object '{}'", name);
        _map[name] = std::move(ptr);
        return false;
    }

    _map.erase(name);
    return true;
}

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

    if (ImGui::Button("Add Component")) {
        ImGui::OpenPopup("AddComponent");
    }

    for (auto& cmp : _map){
        ImGui::PushID(cmp.first.data());
        if (ImGui::CollapsingHeader(cmp.first.data())) {
            auto& p = cmp.second->Position;
            auto& ro = cmp.second->Rotation;
            auto& s = cmp.second->Scale;
            float input_p[3] = {p.x, p.y, p.z};
            float input_r[3] = {(ro.x * 180.0f) / F_PI, (ro.y * 180.0f) / F_PI, (ro.z * 180.0f) / F_PI};
            float input_s[3] = {s.x, s.y, s.z};
            if (ImGui::InputFloat3("Position", input_p, "%.3f")) {
                p = {input_p[0], input_p[1], input_p[2]};
            }
            if (ImGui::InputFloat3("Rotation", input_r)) {
                ro = {(input_r[0] * F_PI) / 180.0f, (input_r[1] * F_PI) / 180.0f,
                                                    (input_r[2] * F_PI) / 180.0f};
            }
            if (ImGui::InputFloat3("Scale", input_s)) {
                s = {input_s[0], input_s[1], input_s[2]};
            }
            cmp.second->UpdateTransform();

            ImGui::Text("Material");
            ImGui::SameLine();
            std::string mat_name = cmp.second->renderObject->MaterialName;
            if (ImGui::Button(mat_name.data())) {
                ImGui::OpenPopup("Material Selector");
            }

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
                        cmp.second->UpdateMesh(name);
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::PopID();
                }
                ImGui::EndPopup();
            }

            if (ImGui::BeginPopup("ShaderPasses")) {
                auto names = _render_resource_map->MaterialMap->GetMaterialList();
                for (auto &name : names) {
                    ImGui::PushID(name.data());
                    if (ImGui::Button(name.data())) {
                        cmp.second->UpdateMaterial(name);
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::PopID();
                }
                ImGui::EndPopup();
            }

            if (ImGui::Button(cmp.second->renderObject->MeshID.c_str())) {
                ImGui::OpenPopup("Meshes");
            }

            if (ImGui::Button("Delete")) {
                DeleteComponent(cmp.first);
            }
        }
        ImGui::PopID();
    }
    ImGui::End();

    /*file browsers*/
    _mesh_file_dialog.Display();
    if (_mesh_file_dialog.HasSelected()) {
        auto selected = _mesh_file_dialog.GetMultiSelected();
        std::vector<std::string> v;
        for (UINT i  = 0; i < selected.size(); i++) {
            APP_LOG_INFO("AsyncLoad Model: {}", selected[i].string());
        }
        _render_resource_map->MeshMap->AsyncLoad(selected);
        _mesh_file_dialog.ClearSelected();
    }
}

void ComponentWindow::OnDestroy() {
};

_END_KONAI3D