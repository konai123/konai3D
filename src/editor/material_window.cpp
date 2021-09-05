//
// Created by korona on 2021-08-07.
//

#include "src/macros.h"
#include "src/editor/material_window.h"
#include "src/k3d_app.h"

_START_KONAI3D
MaterialWindow::MaterialWindow(std::shared_ptr<ViewportWindow> viewportWindow,
                               std::shared_ptr<_ENGINE::Renderer::ResourceMap> resourceMap)
:
IMGUIWindow("Material"),
_render_resource_map(resourceMap),
_viewport_window(viewportWindow),
_file_dialog(ImGuiFileBrowserFlags_MultipleSelection) {
    _file_dialog.SetTitle("Load textures");
    _file_dialog.SetTypeFilters({".jpg", ".dds", ".hdr", ".tga"});
}

bool MaterialWindow::AddMaterial(std::string name) {
    _ENGINE::MaterialDesc newMat = _render_resource_map->MaterialMap->GetMaterialDesc(
            K3DApp::DefaultMaterialName).value();
    if (!_render_resource_map->MaterialMap->AddMaterial(name, newMat)) {
        return false;
    }
    return true;
}

void MaterialWindow::OnUpdate(float delta) {
    auto window_name = GetWindowName();
    if (!ImGui::Begin(window_name.c_str(), &_open, ImGuiWindowFlags_None)) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginPopup("AddMaterial")) {
        static char buf[20] = {0,};
        ImGui::InputText("Mat Name", buf, 20);
        if (ImGui::Button("Ok")) {
            if (!AddMaterial(buf)) {
                APP_LOG_ERROR("Failed to add '{}' material", buf);
            } else {
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }

    if (ImGui::Button("Add Material")) {
        ImGui::OpenPopup("AddMaterial");
    }

    auto material_names = _render_resource_map->MaterialMap->GetMaterialList();
    auto texture_names = _render_resource_map->TextureMap->GetTextureList();
    ImVec2 size = ImGui::GetContentRegionAvail();
    float left_padding = 10.0f;
    auto image_size = ImVec2(size.x - left_padding, 200.0f);

    for (auto &mat_name : material_names) {
        auto material_desc = _render_resource_map->MaterialMap->GetMaterialDesc(mat_name).value();
        auto difffuse_texture = _render_resource_map->TextureMap->GetResource(material_desc.DiffuseTexturePath);
        if (!difffuse_texture.has_value()) continue;

        ImGui::PushID(mat_name.data());

        if (ImGui::CollapsingHeader(mat_name.data())) {
            ImGui::Text("Diffuse Texture");
            if (ImGui::ImageButton(reinterpret_cast<void *>(difffuse_texture.value().Handle.GpuHandle.ptr),
                                   image_size)) {
                ImGui::OpenPopup("Select Diffuse Texture");
            }

            if (ImGui::Combo("Material Type", reinterpret_cast<int *>(&material_desc.MaterialType),
                             "Lambertian\0Metal\0Dielectric\0\0")) {
                _render_resource_map->MaterialMap->UpdateMaterial(mat_name, material_desc);
                _viewport_window->Update();
            }

            if (ImGui::SliderFloat("Fuzz", &material_desc.Fuzz, 0.0f, 1.0f)) {
                _render_resource_map->MaterialMap->UpdateMaterial(mat_name, material_desc);
                _viewport_window->Update();
            }

            if (ImGui::SliderFloat("Index Of Refract", &material_desc.RefractIndex, 1.0f, 5.0f)) {
                _render_resource_map->MaterialMap->UpdateMaterial(mat_name, material_desc);
                _viewport_window->Update();
            }
        }

        if (ImGui::BeginPopup("Select Diffuse Texture")) {
            auto texture_size = ImVec2(400.0f, 200.0f);

            if (ImGui::Button("Load from file")) {
                _file_dialog.Open();
            }

            for (auto &tex_name : texture_names) {
                auto texture = _render_resource_map->TextureMap->GetResource(tex_name);
                if (!texture.has_value()) continue;
                auto texture_id = reinterpret_cast<void *>(texture->Handle.GpuHandle.ptr);
                ImGui::Text(tex_name.data());
                if (ImGui::ImageButton(texture_id, texture_size)) {
                    if (material_desc.DiffuseTexturePath != tex_name) {
                        _viewport_window->Update();
                        material_desc.DiffuseTexturePath = tex_name;
                        _render_resource_map->MaterialMap->UpdateMaterial(mat_name, material_desc);
                    }
                    ImGui::CloseCurrentPopup();
                }
                ImGui::Separator();
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    ImGui::End();

    _file_dialog.Display();
    if (_file_dialog.HasSelected()) {
        auto selected = _file_dialog.GetMultiSelected();
        std::vector<std::string> v;
        for (UINT i = 0; i < selected.size(); i++) {
            APP_LOG_INFO("Load Texture: {}", selected[i].string());
        }
        _render_resource_map->TextureMap->AsyncLoad(selected);
        _file_dialog.ClearSelected();
    }
}

void MaterialWindow::OnDestroy() {
}
_END_KONAI3D
