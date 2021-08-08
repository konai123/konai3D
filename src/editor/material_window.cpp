//
// Created by korona on 2021-08-07.
//

#include "src/macros.h"
#include "src/editor/material_window.h"

_START_KONAI3D
MaterialWindow::MaterialWindow(std::shared_ptr<_ENGINE::Renderer::ResourceMap> resourceMap)
:
IMGUIWindow("Material"),
_render_resource_map(resourceMap),
_file_dialog(ImGuiFileBrowserFlags_MultipleSelection)
{
    _file_dialog.SetTitle("Load textures");
    _file_dialog.SetTypeFilters({".jpg", ".dds", ".hdr", ".tga"});
}

void MaterialWindow::OnUpdate(float delta) {

    auto window_name = GetWindowName();
    if (!ImGui::Begin(window_name.c_str(), &_open, ImGuiWindowFlags_None)) {
        ImGui::End();
        return;
    }

    auto material_names = _render_resource_map->MaterialMap->GetMaterialList();
    auto texture_names = _render_resource_map->TextureMap->GetTextureList();
    ImVec2 size = ImGui::GetContentRegionAvail();
    float left_padding = 10.0f;
    auto image_size = ImVec2(size.x- left_padding, 200.0f);

    for(auto& mat_name : material_names) {
        auto material_desc = _render_resource_map->MaterialMap->GetMaterialDesc(mat_name).value();
        auto difffuse_texture = _render_resource_map->TextureMap->GetResource(material_desc.DiffuseTexturePath);

        ImGui::PushID(mat_name.data());

        if (ImGui::CollapsingHeader(mat_name.data())) {
            ImGui::Text("Diffuse Texture");
            if (ImGui::ImageButton(reinterpret_cast<void *>(difffuse_texture.value().Handle.GpuHandle.ptr), image_size)) {
                ImGui::OpenPopup("Select Diffuse Texture");
            }
        }

        if (ImGui::BeginPopup("Select Diffuse Texture")) {
            auto texture_size = ImVec2(400.0f, 200.0f);

            if (ImGui::Button("Load from file")) {
                _file_dialog.Open();
            }
            for (auto& tex_name : texture_names) {
                auto texture = _render_resource_map->TextureMap->GetResource(tex_name);
                auto texture_id = reinterpret_cast<void*>(texture->Handle.GpuHandle.ptr);
                ImGui::Text(tex_name.data());
                if (ImGui::ImageButton(texture_id, texture_size)) {
                    material_desc.DiffuseTexturePath = tex_name;
                    _render_resource_map->MaterialMap->UpdateMaterial(mat_name, material_desc);
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
        for (UINT i  = 0; i < selected.size(); i++) {
            APP_LOG_INFO("Load Texture: {}", selected[i].string());
        }
        _render_resource_map->TextureMap->AsyncLoad(selected);
        _file_dialog.ClearSelected();
    }
}

void MaterialWindow::OnDestroy() {
}
_END_KONAI3D
