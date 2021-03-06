//
// Created by korona on 2021-06-19.
//

#pragma once

#define _START_KONAI3D      namespace konai3d{
#define _END_KONAI3D        }
#define _USING_KONAI3D      using namespace konai3d;
#define _KONAI3D            ::konai3d

/*assimp headers*/
#include "external/assimp/include/assimp/Importer.hpp"
#include "external/assimp/include/assimp/scene.h"
#include "external/assimp/include/assimp/postprocess.h"

#include "src/engine/pch.h"

/*std headers*/
#include <fstream>

/*windows headers*/

/*imgui-filebrowser headers*/
#include "external/imgui-filebrowser/imfilebrowser.h"

/*ImGuizmo headers*/
#include "external/ImGuizmo/ImGuizmo.h"

/*externals*/
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
