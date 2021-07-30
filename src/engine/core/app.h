//
// Created by korona on 2021-06-09.
//

#ifndef KONAI3D_D3DAPP_H
#define KONAI3D_D3DAPP_H

#include "src/engine/core/timer.h"
#include "src/engine/core/input.h"

_START_ENGINE
class App {
public:
    App();
    virtual ~App() = default;

public:
    virtual bool Prepare(HWND hwnd, int width, int height, float dpiFactor);

public:
    virtual void OnStart();
    virtual void OnDestroy();
    virtual void OnUpdate(float delta);
    virtual void OnLateUpdate(float delta);
    virtual void OnResizeStart(int width, int height);
    virtual void OnResizeEnd();
    virtual void OnDPIUpdate(float dpiFactor);

public:
    void Quit();

public:
    tString GetAppName() const;
    void SetAppName(tString name);
    int GetWidth() const;
    int GetHeight() const;
    int GetMinWidth() const;
    int GetMinHeight() const;
    bool HasAppQuited() const;
    Input *GetInput() const;
    void SetMinWidth(int width);
    void SetMinHeight(int height);
    float GetDPIScaleFactor();

private:
    std::unique_ptr<Input> _input;

    tString _app_name;
    bool _has_quited;
    int _width;
    int _height;
    int _min_width;
    int _min_height;
    float _dpi_scale_factor;
};
_END_ENGINE

#endif //KONAI3D_D3DAPP_H
