cmake_minimum_required(VERSION 3.10)
project(Vesta)

find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(IMGUI_DIR "${EXTERNAL_DIR}/imgui")

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

add_executable(Vesta 
    src/main.cpp
    src/Time.cpp
    src/scene/Sky.cpp
    src/core/Window.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(Vesta PRIVATE
    "src"
    "${EXTERNAL_DIR}/glfw/include"
    "${EXTERNAL_DIR}/glm"
    "${EXTERNAL_DIR}/stb"
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
)

target_link_libraries(Vesta 
    Vulkan::Vulkan
    "${EXTERNAL_DIR}/glfw/lib-mingw-w64/libglfw3.a"
    gdi32
    user32
    kernel32
)

add_custom_command(TARGET Vesta POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:Vesta>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/textures"
        "$<TARGET_FILE_DIR:Vesta>/textures"
)