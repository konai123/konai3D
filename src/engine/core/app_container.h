//
// Created by korona on 2021-06-09.
//

#ifndef KONAI3D_APP_CONTAINER_H
#define KONAI3D_APP_CONTAINER_H

#include "src/engine/core/timer.h"

_START_ENGINE
class App;
class AppContainer {
public:
    using WND_PROC_WITH_CONTAINER = std::add_pointer<LRESULT WINAPI(AppContainer *, HWND, UINT, WPARAM, LPARAM)>::type;

    AppContainer();
    virtual ~AppContainer() = default;

public:
    virtual bool Initiate(int width, int height, std::unique_ptr<App> app, tString exePath,
                          WND_PROC_WITH_CONTAINER wndProc = nullptr);
    virtual void Quit();
    virtual int Run();

public:
    App *GetApp() const;

public:
    static LRESULT WINAPI DefaultWndProc(AppContainer *, HWND, UINT, WPARAM, LPARAM);
    inline static tString ExecutablePath;

public:
    WND_PROC_WITH_CONTAINER _wnd_proc;
    Timer _timer;
    HWND _hwnd;

    bool _app_minimized;
    bool _app_maximized;
    bool _app_resizing;

private:
    std::unique_ptr<App> _app;
};
_END_ENGINE

#endif //KONAI3D_APP_CONTAINER_H
