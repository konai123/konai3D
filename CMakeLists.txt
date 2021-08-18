project(konai3D)

cmake_minimum_required(VERSION 3.1)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

string( TOLOWER "${CMAKE_CXX_COMPILER_ID}" COMPILER_ID )
if(NOT ${COMPILER_ID} MATCHES ".*msvc")
    message( FATAL_ERROR "This project only supports the msvc compiler." )
endif()

#projects
add_subdirectory(external)

add_executable(
        ${CMAKE_PROJECT_NAME}
        src/main.cpp
        src/k3d_app.cpp
        src/global.cpp
        src/imgui_renderer.cpp
        src/camera.cpp

        src/editor/imgui_window.cpp
        src/editor/log_window.cpp
        src/editor/main_window.cpp
        src/editor/viewport_window.cpp
        src/editor/component_window.cpp
        src/editor/material_window.cpp

        src/engine/core/timer.cpp
        src/engine/core/app_container.cpp
        src/engine/core/app.cpp
        src/engine/core/input.cpp
        src/engine/core/pool.hpp
        src/engine/core/rwlock.hpp
        src/engine/core/logger.hpp

        src/engine/graphics/dx_com.cpp
        src/engine/graphics/device_com.cpp
        src/engine/graphics/renderer.cpp
        src/engine/graphics/frame_buffer_pool.cpp
        src/engine/graphics/resource_descriptor_heap.cpp
        src/engine/graphics/utils.hpp
        src/engine/graphics/constant_buffer.cpp
        src/engine/graphics/render_object.cpp
        src/engine/graphics/render_screen.cpp
        src/engine/graphics/sampler.cpp
        src/engine/graphics/upload_buffer.cpp
        src/engine/graphics/render_pass.cpp
        src/engine/graphics/texture_map.cpp
        src/engine/graphics/rw_resource_buffer.cpp
        src/engine/graphics/material_map.cpp
        src/engine/graphics/mesh_map.cpp
        src/engine/graphics/vertex.cpp
        src/engine/graphics/mesh_loader.cpp
        src/engine/graphics/texture_loader.cpp
)

#set_target_properties(${PROJECT_NAME} PROPERTIES
#        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_include_directories(
        ${CMAKE_PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/external/d3d12/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/assimp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/DirectXTex/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/DirectXTK12/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog/include
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui-filebrowser
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(
        ${CMAKE_PROJECT_NAME}
        PRIVATE
        src/pch.h
        src/engine/pch.h
)

target_link_directories(${CMAKE_PROJECT_NAME}
        PRIVATE
        external/assimp/lib
)

set(AGILITYD3D_PATH external/d3d12/lib/x64)

#linking
target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        PRIVATE
        d3d12
        d3dcompiler
        dxgi
        dxguid
        imgui
        ShCore
        assimp-vc142-mt.lib
        ImGuizmo
)

target_link_libraries(
        ${CMAKE_PROJECT_NAME}
        PRIVATE
        debug
        ${CMAKE_CURRENT_SOURCE_DIR}/external/DirectXTex/lib/Debug/DirectXTex.lib
        debug
        ${CMAKE_CURRENT_SOURCE_DIR}/external/DirectXTK12/lib/Debug/DirectXTK12.lib
        optimized
        ${CMAKE_CURRENT_SOURCE_DIR}/external/DirectXTex/lib/Release/DirectXTex.lib
        optimized
        ${CMAKE_CURRENT_SOURCE_DIR}/external/DirectXTK12/lib/Release/DirectXTK12.lib
)

add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/external/assimp/lib/assimp-vc142-mt.dll
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>
        DEPENDS ${CMAKE_PROJECT_NAME}
)

add_custom_command (
        TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/${AGILITYD3D_PATH}"
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/D3D12
)

add_custom_command (
        TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/assets
)

add_custom_command (
        TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/shaders
)