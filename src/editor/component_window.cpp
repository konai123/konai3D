//
// Created by korona on 2021-07-30.
//

#include "src/editor/component_window.h"

#include "src/macros.h"
#include "src/vertex.h"
#include "src/math.h"
#include "src/basic_shader_pass_attrbute.hpp"

_START_KONAI3D
ComponentWindow::ComponentWindow(
        std::shared_ptr<_ENGINE::RenderScreen> screen,
        std::shared_ptr<ShaderPassMap> shaderPassMap,
        std::shared_ptr<MeshMap> meshMap,
        std::weak_ptr<_ENGINE::Renderer> renderer
)
:
IMGUIWindow("Components"),
_screen(screen),
_shader_pass_map(shaderPassMap),
_mesh_map(meshMap),
_renderer(renderer),
_window_flags(ImGuiWindowFlags_MenuBar),
_mesh_file_dialog(ImGuiFileBrowserFlags_MultipleSelection) {
    _mesh_file_dialog.SetTitle("Load Mesh");
    _mesh_file_dialog.SetTypeFilters({".fbx", ".obj"});
}

bool ComponentWindow::AddComponent(std::string name) {
    if (_screen->GetRenderObject(name) != nullptr) return false;
    auto renderer_ptr = _renderer.lock();
    if (renderer_ptr == nullptr)
        return false;

    auto mesh = _mesh_map->GetDrawInfo(_mesh_map->_default_mesh);
    auto shader_pass = _shader_pass_map->GetShaderPass(_shader_pass_map->_default_shader_pass);
    auto render_obj = std::make_shared<_ENGINE::RenderObject>(std::move(shader_pass), std::move(mesh), name);

    auto cb_buffer = renderer_ptr->InstanceConstanceBuffer();
    AppAssert(cb_buffer != nullptr);

    render_obj->SetPosition(DirectX::XMVectorSet(0, 0, 0, 1.0f));
    render_obj->SetRotation(DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));
    render_obj->SetScale(DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));

    BasicShaderPassAttribute::PerObject obj;
    DirectX::XMStoreFloat4x4(&obj.world_mat, render_obj->GetWorldMatrix());
    if (!cb_buffer->SetData(&obj, sizeof(BasicShaderPassAttribute::PerObject))) {
        APP_LOG_ERROR("Failed to instance constant buffer, cannot add component {}", name);
        return false;
    }
    render_obj->AddConstantBuffer(BasicShaderPassAttribute::_per_object, std::move(cb_buffer));
    UpdateRenderObjectConstant(render_obj.get());

    _shader_pass_names[name] = _shader_pass_map->_default_shader_pass;
    _mesh_names[name] = _mesh_map->_default_mesh;
    return _screen->AddRenderObject(std::move(render_obj));
}

bool ComponentWindow::DeleteComponent(std::string name) {
    if (_screen->DeleteRenderObject(name) == false) {
        APP_LOG_ERROR("Failed to delete render object '{}'", name);
        return false;
    }
    _shader_pass_names.erase(name);
    _mesh_names.erase(name);
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

    auto renderObjs = _screen->GetRenderObjects();
    for (UINT i = 0; i < renderObjs.size(); i++) {
        auto &r = renderObjs[i];
        ImGui::PushID(r->GetName().data());
        if (ImGui::CollapsingHeader(r->GetName().data())) {
            auto p = r->GetPosition();
            auto ro = r->GetRotation();
            auto s = r->GetScale();
            float input_p[3] = {p.m128_f32[0], p.m128_f32[1], p.m128_f32[2]};
            float input_r[3] = {(ro.m128_f32[0] * 180.0f) / F_PI, (ro.m128_f32[1] * 180.0f) / F_PI,
                                (ro.m128_f32[2] * 180.0f) / F_PI};
            float input_s[3] = {s.m128_f32[0], s.m128_f32[1], s.m128_f32[2]};
            if (ImGui::InputFloat3("Position", input_p, "%.3f")) {
                r->SetPosition(DirectX::XMVectorSet(input_p[0], input_p[1], input_p[2], 1.0f));
            }
            if (ImGui::InputFloat3("Rotation", input_r)) {
                r->SetRotation(DirectX::XMVectorSet((input_r[0] * F_PI) / 180.0f, (input_r[1] * F_PI) / 180.0f,
                                                    (input_r[2] * F_PI) / 180.0f, 1.0f));
            }
            if (ImGui::InputFloat3("Scale", input_s)) {
                r->SetScale(DirectX::XMVectorSet(input_s[0], input_s[1], input_s[2], 1.0f));
            }
            UpdateRenderObjectConstant(r);

            ImGui::Text("Shader Pass: ");
            ImGui::SameLine();
            if (ImGui::Button(_shader_pass_names[r->GetName()].data())) {
                ImGui::OpenPopup("ShaderPasses");
            }

            ImGui::Text("Mesh: ");
            ImGui::SameLine();

            if (ImGui::BeginPopup("Meshes")) {
                auto names = _mesh_map->GetMeshList();
                if (ImGui::Button("From Files...")) {
                    _mesh_file_dialog.Open();
                }

                for (auto &name : names) {
                    ImGui::PushID(name.data());
                    if (ImGui::Button(name.data())) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::PopID();
                }
                ImGui::EndPopup();
            }

            if (ImGui::BeginPopup("ShaderPasses")) {
                auto names = _shader_pass_map->GetShaderPasses();
                for (auto &name : names) {
                    ImGui::PushID(name.data());
                    if (ImGui::Button(name.data())) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::PopID();
                }
                ImGui::EndPopup();
            }

            if (ImGui::Button(_mesh_names[r->GetName()].data())) {
                ImGui::OpenPopup("Meshes");
            }

            if (ImGui::Button("Delete")) {
                DeleteComponent(r->GetName());
            }
        }
        ImGui::PopID();
    }
    ImGui::End();

    /*file browsers*/
    _mesh_file_dialog.Display();
    if (_mesh_file_dialog.HasSelected()) {
        auto selected = _mesh_file_dialog.GetMultiSelected();
        for (UINT i  = 0; i < selected.size(); i++) {
            APP_LOG_INFO(selected[i].string());
        }
        _mesh_file_dialog.ClearSelected();
    }
}

void ComponentWindow::OnDestroy() {
};

void ComponentWindow::UpdateRenderObjectConstant(_ENGINE::RenderObject *obj) {
    BasicShaderPassAttribute::PerObject per_object;
    DirectX::XMStoreFloat4x4(&per_object.world_mat, obj->GetWorldMatrix());
    auto renderer_ptr = _renderer.lock();
    if (renderer_ptr != nullptr) {
        obj->UpdateConstantBuffer(BasicShaderPassAttribute::_per_object, &per_object,
                                  renderer_ptr->GetCurrentFrameIndex());
    }
}
_END_KONAI3D