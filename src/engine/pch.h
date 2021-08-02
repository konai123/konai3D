//
// Created by korona on 2021-06-07.
//

#ifndef KONAI3D_PCH_H
#define KONAI3D_PCH_H

#define _START_ENGINE      namespace engine{
#define _END_ENGINE        }
#define _USING_ENGINE      using namespace engine;
#define _ENGINE            ::engine

/*std headers*/
#include <iostream>
#include <optional>
#include <cstdio>
#include <string>
#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <utility>
#include <future>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <filesystem>
#include <queue>
#include <map>
#include <variant>
#include <set>

/*windows headers*/
#include <system_error>
#include <wrl.h>
#include <comdef.h>
#include <Windows.h>
#include <windowsx.h>
#include <WinUser.h>
#include <hidusage.h>
#include <tchar.h>
#include <synchapi.h>
#include <ShellScalingApi.h>

/*d3d headers*/
#include "src/d3d_definition.h"

/*IMGUI header*/
#include "external/imgui/imgui.h"
#include "external/imgui/backends/imgui_impl_win32.h"
#include "external/imgui/backends/imgui_impl_dx12.h"

/*spdlog*/
#include "external/spdlog/include/spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/base_sink.h"

/*json*/
#include "external/json/include/json.hpp"

/*types*/
typedef std::wstring tString;

// Additional VK Definitions
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A

#define float4x4 DirectX::XMFLOAT4X4

#define float2 DirectX::XMFLOAT2
#define float3 DirectX::XMFLOAT3
#define float4 DirectX::XMFLOAT4

#define int2 DirectX::XMINT2
#define int3 DirectX::XMINT3
#define int4 DirectX::XMINT4


#endif //KONAI3D_PCH_H
