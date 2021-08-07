//
// Created by korona on 2021-07-05.
//

#pragma once

#include "src/editor/imgui_window.h"
#include "src/engine/graphics/render_object.h"
#include "src/engine/graphics/mesh_map.h"
#include "src/engine/graphics/material_map.h"

_START_KONAI3D

struct Component {
public:
    Component(std::string materialName, std::string meshName) {
        renderObject = _ENGINE::RenderObject::AllocRenderObject();
        renderObject->MaterialName = materialName;
        renderObject->MeshID = meshName;
        UpdateTransform();
    }

    virtual ~Component() {
        _ENGINE::RenderObject::DiscardRenderObject(renderObject);
    }

public:
    void UpdateTransform() {
        DirectX::XMMATRIX ts = {
                Scale.x, 0, 0, 0,
                0, Scale.y, 0, 0,
                0, 0, Scale.z, 0,
                Position.x, Position.y, Position.z, 1
        };

        DirectX::XMMATRIX rot = DirectX::XMMatrixRotationRollPitchYaw(Rotation.x, Rotation.y, Rotation.z);
        DirectX::XMStoreFloat4x4(&renderObject->WorldMatrix, DirectX::XMMatrixMultiply(ts, rot));
    }

    void UpdateMaterial (std::string materialName) {
        renderObject->MaterialName = materialName;
    }

    void UpdateMesh (std::string meshName) {
        renderObject->MeshID = meshName;
    }

public:
    float3 Position = {0.0f, 0.0f, 0.0f};
    float3 Rotation = {0.0f, 0.0f, 0.0f};
    float3 Scale = {1.0f, 1.0f, 1.0f};

    _ENGINE::RenderObject* renderObject;
};

class ComponentWindow : public IMGUIWindow {
public:
    ComponentWindow(
            std::shared_ptr<_ENGINE::RenderScreen> screen,
            std::shared_ptr<_ENGINE::Renderer::ResourceMap> resourceMap
    );
    virtual ~ComponentWindow() = default;

public:
    bool AddComponent(std::string name);
    bool DeleteComponent(std::string name);

public:
    virtual void OnUpdate(float delta) override;
    virtual void OnDestroy() override;

private:
    ImGuiWindowFlags _window_flags;
    ImGui::FileBrowser _mesh_file_dialog;
    std::shared_ptr<_ENGINE::RenderScreen> _screen;

    std::unordered_map<std::string, std::unique_ptr<Component>> _map;
    std::shared_ptr<_ENGINE::Renderer::ResourceMap> _render_resource_map;
};
_END_KONAI3D
