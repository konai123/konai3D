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
    auto newRenderObj = _ENGINE::RenderObject::AllocRenderObject();
    newRenderObj->MaterialName = K3DApp::DefaultMaterialName;
    newRenderObj->MeshID = K3DApp::DefaultMeshName;

    if (!_screen->AddRenderObject(name, newRenderObj)) {
        return false;
    }
    return true;
}

bool ComponentWindow::DeleteComponent(std::string name) {
    if (!_screen->UnRegisterRenderObject(name)) {
        APP_LOG_ERROR("Failed to delete render object '{}'", name);
        return false;
    }
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

    auto names = _screen->GetRenderObjectList();
    for(auto& name : names) {
        auto cmp = _screen->GetRenderObject(name);
        ImGui::PushID(name.data());
        if (ImGui::CollapsingHeader(name.data())) {
            auto& p = cmp->Position;
            auto& ro = cmp->Rotation;
            auto& s = cmp->Scale;
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
            cmp->UpdateTransform();

            ImGui::Text("Material: ");
            ImGui::SameLine();
            std::string mat_name = cmp->MaterialName;
            if (ImGui::Button(mat_name.data())) {
                ImGui::OpenPopup("Material Selector");
            }

            auto mat_desc = _render_resource_map->MaterialMap->GetMaterialDesc(mat_name);
            auto texture_resource = _render_resource_map->TextureMap->GetResource(mat_desc->DiffuseTexturePath);
            if (texture_resource) {
                ImVec2 image_size(100.0f, 100.0f);
                ImGui::Image(reinterpret_cast<void *>(texture_resource.value().Handle.GpuHandle.ptr), image_size);
            }else {
                ImGui::Text("Texture load failed.");
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
                        cmp->UpdateMesh(name);
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