# Voxel Renderer

[![build](https://github.com/jswiatly/Voxel-Renderer/actions/workflows/build.yml/badge.svg)](https://github.com/jswiatly/Voxel-Renderer/actions/workflows/build.yml)

A real-time renderer for procedural voxel terrain, written from scratch in **C++20** and **Vulkan 1.3**.

`C++20` · `Vulkan 1.3` · `GLFW` · `GLM` · `Dear ImGui` · `VMA`

![demo](docs/demo.gif)

---

## Features

**Rendering**

- Hand-written Vulkan 1.3 renderer — instance, device, swapchain, graphics pipeline, built from the ground up.
- GPU memory managed through VMA (Vulkan Memory Allocator).
- Depth buffering; mipmapped textures with anisotropic filtering.
- Automatic swapchain recreation on resize; prefers `MAILBOX` present mode with a `FIFO` fallback.
- Directional lighting with per-face normals, driven by the sun's position in the day/night cycle.

**Tooling / debug**

- Free-fly camera (WASD + mouse-look).
- Dear ImGui panels: frame-time/FPS overlay, 3D orientation gizmo, time-of-day slider, and an in-engine viewer for Vulkan validation-layer messages.
- VMA allocation statistics dumped to JSON.

## Architecture

Single-responsibility modules behind a thin `Engine` facade (`main.cpp` is ~15 lines):

| Module             | Responsibility                                                        |
| ------------------ | --------------------------------------------------------------------- |
| `VulkanContext`    | instance, device, queues, allocator, command pool, shared GPU helpers |
| `Swapchain`        | swapchain, image views, depth buffer, framebuffers, resize recreation |
| `Pipeline`         | render pass, descriptor layout, graphics pipeline, shaders            |
| `Texture`          | image upload, mipmap generation, sampler                              |
| `Mesh`             | vertex / index / uniform buffers, descriptor sets                     |
| `Renderer`         | command buffers, synchronization, draw loop                           |
| `ImGuiLayer`       | Dear ImGui setup and debug UI                                         |
| `InputHandler`     | keyboard / mouse → camera                                             |
| `ValidationLogger` | captures Vulkan validation messages for the in-engine viewer          |

## Tech stack

| Area              | Library    |
| ----------------- | ---------- |
| Graphics API      | Vulkan 1.3 |
| Windowing / input | GLFW       |
| Math              | GLM        |
| GPU memory        | VMA        |
| UI / debug        | Dear ImGui |
| Image loading     | stb_image  |

All dependencies except the Vulkan SDK are fetched automatically by CMake — no manual setup.

## Building

Requirements:

- [Vulkan SDK](https://vulkan.lunarg.com/) (includes the `glslc` shader compiler)
- CMake 3.24+
- A C++20 compiler (MSVC, GCC/MinGW, or Clang)

```sh
git clone https://github.com/jswiatly/Voxel-Renderer.git
cd Voxel-Renderer
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

On Linux, GLFW additionally needs the windowing-system headers:

```sh
sudo apt install xorg-dev libwayland-dev libxkbcommon-dev wayland-protocols
```

## Controls

| Input           | Action                |
| --------------- | --------------------- |
| `W` `A` `S` `D` | Move camera           |
| Mouse           | Look around           |
| `F`             | Toggle cursor capture |

## Roadmap

- Chunked world with multithreaded chunk meshing (generation/meshing off the render thread).
- Greedy meshing to reduce vertex counts.
- Per-chunk frustum culling + backface culling.
- Shadow mapping tied to the sun direction.
- Block editing (add / remove voxels).

## License

MIT — see [LICENSE](LICENSE).
