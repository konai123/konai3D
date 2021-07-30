//
// Created by khhan on 2021-06-18.
//

#ifndef KONAI3D_IMGUI_WINDOW_H
#define KONAI3D_IMGUI_WINDOW_H

#include "src/engine/graphics/renderer.h"

_START_KONAI3D
class IMGUIWindow {
public:
    IMGUIWindow(std::string name);
    virtual ~IMGUIWindow() = default;

public:
    virtual void OnUpdate(float delta);
    virtual void OnDestroy();
    virtual std::string GetWindowName();
    virtual bool IsCollapsed();
    virtual void Open(bool state);

public:
    void Collapse(bool state);
    void Name(std::string name);

protected:
    std::string _window_name;
    bool _open = false;
};
_END_KONAI3D

#endif //KONAI3D_IMGUI_WINDOW_H
