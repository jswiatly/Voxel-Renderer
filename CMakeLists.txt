cmake_minimum_required(VERSION 3.10)
project(VulkanTest)

find_package(Vulkan REQUIRED)

add_executable(Vesta main.cpp)

target_include_directories(Vesta PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/glfw/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}/stb"
)

target_link_libraries(Vesta 
    Vulkan::Vulkan
    "${CMAKE_CURRENT_SOURCE_DIR}/glfw/lib-mingw-w64/libglfw3.a"
    gdi32
    user32
    kernel32
)