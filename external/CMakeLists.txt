project(external)

add_subdirectory(imgui)
add_subdirectory(ImGuizmo)

target_include_directories(ImGuizmo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/imgui)