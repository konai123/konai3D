cmake_minimum_required(VERSION 3.1)
project(imgui)

add_library(
        ${PROJECT_NAME}
        imgui.cpp
        imgui_draw.cpp
        imgui_tables.cpp
        imgui_widgets.cpp
        imgui_demo.cpp
        backends/imgui_impl_dx12.cpp
        backends/imgui_impl_win32.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})