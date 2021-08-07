//
// Created by korona on 2021-06-09.
//

#include "src/engine/core/app_container.h"
#include "src/engine/core/app.h"
#include "src/engine/core/macros.h"

_START_ENGINE
AppContainer::AppContainer()
:
        Hwnd(nullptr),
        AppMinimized(false),
        AppMaximized(false),
        AppResizing(false),
        _app(nullptr) {
    WndProc = DefaultWndProc;
}

bool AppContainer::Initiate(int width, int height, std::unique_ptr<App> app, tString exePath,
                            WND_PROC_WITH_CONTAINER wndProc) {
    ExecutablePath = exePath;
    if (app == nullptr) {
        return false;
    }
    ::SetProcessDpiAwareness(PROCESS_DPI_AWARENESS::PROCESS_PER_MONITOR_DPI_AWARE);

    _app = std::move(app);
    tString window_name = _app->GetAppName();
    if (wndProc != nullptr) WndProc = wndProc;

    /*
     * WndProc Decorated With Lambda expression To pass instance.
     * Therefore, some messages before window is completely completed
     * are not received to user defined wndproc[defaultproc]
     * */
    auto with_wndproc = [](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
        AppContainer *context = reinterpret_cast<AppContainer *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (context == nullptr)
            return ::DefWindowProc(hWnd, msg, wParam, lParam);
        return context->WndProc(context, hWnd, msg, wParam, lParam);
    };

    WNDCLASSEXW wc = {
            sizeof(WNDCLASSEX), CS_CLASSDC, static_cast<WNDPROC>(with_wndproc), 0L, 0L,
            GetModuleHandle(nullptr), nullptr, nullptr, nullptr,
            nullptr, window_name.c_str(), nullptr
    };

    ::RegisterClassExW(&wc);
    Hwnd = ::CreateWindowW(wc.lpszClassName, window_name.c_str(), WS_OVERLAPPEDWINDOW,
                           100, 100, width, height, nullptr, nullptr, wc.hInstance, nullptr);

    ::SetWindowLongPtr(Hwnd, GWLP_USERDATA, (LONG_PTR) this);

    if (Hwnd == nullptr) {
        CORE_LOG_INFO("Failed to create window.");
        return false;
    }

    /*
     * Getting Monitors dpi factor.
     * */
    UINT dpi = GetDpiForWindow(Hwnd);
    float dpiFactor = dpi / static_cast<float>(USER_DEFAULT_SCREEN_DPI);

    if (!_app->Prepare(Hwnd, width, height, dpiFactor)) {
        CORE_LOG_ERROR("Failed to prepare application.");
        return false;
    }
    return true;
}

void AppContainer::Quit() {
    ::PostQuitMessage(0);
}

int AppContainer::Run() {
    EngineAssert(_app.get());
    MSG msg = {0};
    Timer.Reset();
    _app->OnStart();
    ::ShowWindow(Hwnd, SW_SHOW);
    ::UpdateWindow(Hwnd);

    while (msg.message != WM_QUIT) {
        if (_app->HasAppQuited()) Quit();
        if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            _app->GetInput()->HandleMessage(&msg);
        } else {
            Timer.Tick();

            float delta = Timer.DeltaTime();
            _app->OnUpdate(delta);
            _app->OnLateUpdate(delta);
        }
    }

    _app->OnDestroy();
    ::DestroyWindow(Hwnd);
    return static_cast<int>(msg.wParam);
}

App *AppContainer::GetApp() const {
    return _app.get();
}

LRESULT
WINAPI AppContainer::DefaultWndProc(AppContainer *appContainer, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            appContainer->GetApp()->OnResizeStart(width, height);
            if (wParam == SIZE_MINIMIZED) {
                appContainer->AppMinimized = true;
                appContainer->AppMaximized = false;
            } else if (wParam == SIZE_MAXIMIZED) {
                appContainer->AppMinimized = false;
                appContainer->AppMaximized = true;
                appContainer->GetApp()->OnResizeEnd();
            } else if (wParam == SIZE_RESTORED) {
                if (appContainer->AppMinimized) appContainer->AppMinimized = false;
                if (appContainer->AppMaximized) appContainer->AppMaximized = false;
                if (appContainer->AppResizing) {
                    return 0;
                }
                appContainer->GetApp()->OnResizeEnd();
            }
            return 0;
        }
        case WM_ENTERSIZEMOVE:
            appContainer->AppResizing = true;
            appContainer->Timer.Stop();
            return 0;
        case WM_EXITSIZEMOVE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            appContainer->AppResizing = false;
            appContainer->GetApp()->OnResizeEnd();
            appContainer->Timer.Start();
            return 0;
        }
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        case WM_MENUCHAR:
            return MAKELRESULT(0, MNC_CLOSE);
        case WM_GETMINMAXINFO:
            ((MINMAXINFO *) lParam)->ptMinTrackSize.x = appContainer->GetApp()->GetMinWidth();
            ((MINMAXINFO *) lParam)->ptMinTrackSize.y = appContainer->GetApp()->GetMinHeight();
            return 0;

        case WM_DPICHANGED: {
            float dpi_scale_factor = HIWORD(wParam) / static_cast<float>(USER_DEFAULT_SCREEN_DPI);
            appContainer->GetApp()->OnDPIUpdate(dpi_scale_factor);
        }
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
_END_ENGINE

