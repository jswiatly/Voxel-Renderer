# Vesta

[![build](https://github.com/jswiatly/Vesta-Engine/actions/workflows/build.yml/badge.svg)](https://github.com/jswiatly/Vesta-Engine/actions/workflows/build.yml)

A voxel engine built from scratch in modern **C++20** and **Vulkan 1.3**.

`C++20` · `Vulkan 1.3` · `GLFW` · `GLM` · `Dear ImGui` · `VMA`

<!--
 ![Vesta demo](docs/demo.gif)
-->

> A small renderer exploring how far a modern C++ / Vulkan codebase can push voxel
> rendering on current hardware.

---

## Features

These are implemented and working today.

**Rendering**
- Hand-written **Vulkan 1.3** renderer (instance, device, swapchain, pipeline) built from the ground up.
- Buffer and image memory managed through **VMA** (Vulkan Memory Allocator) instead of manual `VkDeviceMemory` juggling.
- **Depth buffering** for correct occlusion.
- **Mipmapped textures** with anisotropic filtering.
- Procedural **voxel scene generation**.
- **Double-buffered frame pacing** (2 frames in flight) with explicit semaphore/fence synchronization.
- Automatic **swapchain recreation** on window resize.
- Prefers `MAILBOX` present mode with a `FIFO` fallback.

**Engine**
- Free-fly **camera** — WASD movement with mouse-look.
- **Day/night cycle** driving a dynamic sky colour over an adjustable in-game clock.

**Tooling / debug**
- **Dear ImGui** integration with several live panels:
  - performance overlay (ms/frame and FPS),
  - a 3D **orientation gizmo** showing camera pitch/yaw,
  - a manual time-of-day slider,
  - an in-engine **validation-layer log viewer** (Vulkan validation messages are captured and shown inside the app, not just dumped to the console).
- **VMA memory-statistics dump** to JSON for inspecting allocations.

---

## Tech stack

| Area | Library |
|------|---------|
| Graphics API | Vulkan 1.3 |
| Windowing / input | GLFW |
| Math | GLM |
| GPU memory | VMA (Vulkan Memory Allocator) |
| UI / debug | Dear ImGui |
| Image loading | stb_image |

Language standard: **C++20**.

---

## Controls

| Input | Action |
|-------|--------|
| `W` `A` `S` `D` | Move camera |
| Mouse | Look around |
| `F` | Toggle cursor capture (free the mouse for ImGui) |

---

## Building

Requirements:

- [Vulkan SDK](https://vulkan.lunarg.com/) (includes the `glslc` shader compiler)
- CMake 3.24+
- A C++20 compiler (MSVC, GCC/MinGW, Clang)

All other dependencies (GLFW, GLM, Dear ImGui, VMA) are fetched automatically
by CMake at configure time - no manual setup needed.

```sh
git clone https://github.com/jswiatly/Vesta-Engine.git
cd Vesta-Engine
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

On Linux, GLFW additionally needs the windowing-system headers:

```sh
sudo apt install xorg-dev libwayland-dev libxkbcommon-dev wayland-protocols
```

Run `Vesta` from the build directory (shaders and assets are copied next to
the executable at build time).

## Roadmap

Planned directions, not yet implemented:

- **Ray tracing** for global illumination / reflections.
- **Large terrain render distance** via chunk streaming.
- Migrate the rendering path from a fixed `VkRenderPass` to **dynamic rendering** (already enabled at device level).
- **Cross-platform builds**: Windows x86, Linux x86/ARM, macOS ARM (via MoltenVK).
- Energy-aware scaling for handheld targets (e.g. Steam Deck, Apple Silicon).

---

## Motivation

Voxel games are usually remembered as gameplay, rarely as rendering. But the genre is
quietly one of the most demanding things you can ask a renderer to do - enormous,
fully dynamic worlds where geometry changes constantly and the view distance is never
big enough.

Most existing voxel engines were built on older stacks (Java + OpenGL), and they leave a lot of modern hardware on the table: large L3 caches,
high core counts, and ray-tracing-capable GPUs that mostly sit idle. Projects like the ray-traced builds of
Minecraft: Bedrock Edition showed how good the genre *can* look before that work stalled.

Vesta is my attempt to start from a clean, modern foundation - C++20 and Vulkan 1.3 -
and see how much performance and visual range that foundation actually unlocks when the
hardware is treated as a first-class target. It's also how I'm learning real-time graphics.
