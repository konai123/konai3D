//
// Created by korona on 2021-06-19.
//

#ifndef PCH_H
#define PCH_H

#define _START_KONAI3D      namespace konai3d{
#define _END_KONAI3D        }
#define _USING_KONAI3D      using namespace konai3d;
#define _KONAI3D            ::konai3d

/*assimp*/
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "src/engine/pch.h"

/*std headers*/

/*windows headers*/

/*externals*/
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif //PCH_H
