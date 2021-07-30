//
// Created by korona on 2021-07-05.
//

#ifndef KONAI3D_COMPONENT_WINDOW_H
#define KONAI3D_COMPONENT_WINDOW_H

#include "src/editor/imgui_window.h"
#include "src/engine/graphics/render_object.h"
#include "src/shader_pass_map.h"
#include "src/mesh_map.h"

_START_KONAI3D
class ComponentWindow : public IMGUIWindow {
public:
    ComponentWindow(
            std::shared_ptr<_ENGINE::RenderScreen> screen,
            std::shared_ptr<ShaderPassMap> shaderPassMap,
            std::shared_ptr<MeshMap> meshMap,
            std::weak_ptr<_ENGINE::Renderer> renderer
    );
    virtual ~ComponentWindow() = default;

public:
    bool AddComponent(std::string name);
    bool DeleteComponent(std::string name);

public:
    virtual void OnUpdate(float delta) override;
    virtual void OnDestroy() override;

private:
    void UpdateRenderObjectConstant(_ENGINE::RenderObject *obj);

private:
    ImGuiWindowFlags _window_flags;
    std::shared_ptr<_ENGINE::RenderScreen> _screen;
    std::shared_ptr<ShaderPassMap> _shader_pass_map;
    std::shared_ptr<MeshMap> _mesh_map;
    std::unordered_map<std::string, std::string> _shader_pass_names;
    std::unordered_map<std::string, std::string> _mesh_names;
    std::weak_ptr<_ENGINE::Renderer> _renderer;
};
_END_KONAI3D

#endif //KONAI3D_COMPONENT_WINDOW_H
