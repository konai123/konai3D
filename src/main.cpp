//
// Created by korona on 2021-06-05.
//

#include "src/engine/core/timer.h"
#include "src/engine/core/app.h"
#include "src/engine/core/app_container.h"

#include "src/k3d_app.h"
#include "src/global.h"

namespace {
    LRESULT WINAPI WndProc(_ENGINE::AppContainer* ctx, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
        return _ENGINE::AppContainer::DefaultWndProc(ctx, hWnd, msg, wParam, lParam);
    }
}

_USING_KONAI3D
int main() {
#if defined(_DEBUG) || defined(DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetBreakAlloc(-1);
#endif

    int argc;
    LPWSTR* argv = ::CommandLineToArgvW(GetCommandLineW(), &argc);
    std::filesystem::path exe_path{ argv[0] };
    global::ExecutablePath = exe_path.parent_path();
    global::AssetPath = global::ExecutablePath / "assets";

#if defined(_DEBUG) || defined(DEBUG)
    global::ShaderPath = std::filesystem::path(SOURCE_ROOT) / "shaders";
#else
    global::ShaderPath = global::ExecutablePath / "shaders";
#endif
    global::ScreenPath = global::AssetPath / "screens";
    LocalFree(argv);

    std::unique_ptr<_ENGINE::App> app = std::make_unique<K3DApp>();
    app->SetAppName(_T("konai3D"));

    auto container = std::make_unique<_ENGINE::AppContainer>();
    if (!container->Initiate(1920, 1080, std::move(app), exe_path.wstring(), WndProc))
        return -1;
    int retv = container->Run();
    container.reset();

#if defined(DEBUG) || defined(_DEBUG)
    IDXGIDebug1* pDebug = nullptr;
    if (SUCCEEDED(::DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
    {
        pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
        pDebug->Release();
    }
#endif

    return retv;
}