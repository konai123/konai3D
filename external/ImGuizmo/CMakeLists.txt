cmake_minimum_required(VERSION 3.1)
project(ImGuizmo)

add_library(
        ${PROJECT_NAME}
        GraphEditor.cpp
        ImCurveEdit.cpp
        ImGradient.cpp
        ImGuizmo.cpp
        ImSequencer.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})