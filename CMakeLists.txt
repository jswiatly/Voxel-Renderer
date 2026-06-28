cmake_minimum_required(VERSION 3.24)
project(VoxelRenderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED COMPONENTS glslc)

# ---------------------------------------------------------------------------
# Dependencies (fetched automatically at configure time)
# ---------------------------------------------------------------------------
include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.8-docking
)
FetchContent_Declare(VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(glfw glm imgui VulkanMemoryAllocator)

# Dear ImGui ships no CMakeLists, so build it as a small static library.
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)

# ---------------------------------------------------------------------------
# Engine executable
# ---------------------------------------------------------------------------
add_executable(VoxelRenderer
    src/main.cpp
    src/Time.cpp
    src/core/Window.cpp
    src/core/Debug.cpp
    src/core/VulkanContext.cpp
    src/renderer/Vertex.cpp
    src/scene/Sky.cpp
    src/scene/Terrain.cpp
    src/core/Swapchain.cpp
    src/core/Texture.cpp
    src/core/InputHandler.cpp
    src/core/Pipeline.cpp
    src/core/Mesh.cpp
    src/core/ImGuiLayer.cpp
    src/core/Renderer.cpp
    src/core/Engine.cpp
)

target_include_directories(VoxelRenderer PRIVATE src)

target_link_libraries(VoxelRenderer PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
    imgui
    GPUOpen::VulkanMemoryAllocator
)

# ---------------------------------------------------------------------------
# Shaders (compiled with glslc from the Vulkan SDK) and runtime assets
# ---------------------------------------------------------------------------
set(SHADER_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

add_custom_command(
    OUTPUT ${SHADER_OUT_DIR}/vert.spv
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUT_DIR}
    COMMAND Vulkan::glslc ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert -o ${SHADER_OUT_DIR}/vert.spv
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert
    COMMENT "Compiling shader.vert"
)
add_custom_command(
    OUTPUT ${SHADER_OUT_DIR}/frag.spv
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUT_DIR}
    COMMAND Vulkan::glslc ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag -o ${SHADER_OUT_DIR}/frag.spv
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag
    COMMENT "Compiling shader.frag"
)
add_custom_target(Shaders DEPENDS ${SHADER_OUT_DIR}/vert.spv ${SHADER_OUT_DIR}/frag.spv)
add_dependencies(VoxelRenderer Shaders)

add_custom_command(TARGET VoxelRenderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SHADER_OUT_DIR}
        $<TARGET_FILE_DIR:VoxelRenderer>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/textures
        $<TARGET_FILE_DIR:VoxelRenderer>/textures
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/fonts
        $<TARGET_FILE_DIR:VoxelRenderer>/fonts
)
