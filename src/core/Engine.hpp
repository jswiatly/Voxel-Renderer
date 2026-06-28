#pragma once

#include <glm/glm.hpp>
#include <vk_mem_alloc.h>

#include "Time.hpp"
#include "core/Constants.hpp"
#include "core/Debug.hpp"
#include "core/ImGuiLayer.hpp"
#include "core/InputHandler.hpp"
#include "core/Mesh.hpp"
#include "core/Pipeline.hpp"
#include "core/Renderer.hpp"
#include "core/Swapchain.hpp"
#include "core/Texture.hpp"
#include "core/VulkanContext.hpp"
#include "core/Window.hpp"
#include "scene/Camera.hpp"

class Engine {
  public:
    Engine(int width, int height) : window_(width, height, "Voxel Renderer") {}
    void run();

  private:
    void initVulkan();
    void mainLoop();
    void cleanup();
    void DumpVMAMemoryStats(VmaAllocator allocator, const char* filename);

    Window window_;
    Camera camera;
    ValidationLogger m_validationLog;
    vkr::Time time;
    VulkanContext m_context;
    Swapchain m_swapchain;
    Texture m_texture;
    InputHandler m_input;
    Pipeline m_pipeline;
    std::vector<Mesh> m_chunks;
    ImGuiLayer m_imgui;
    Renderer m_renderer;

    float m_timeOfDay = 0.0f;
    bool m_manualTime = false;
    float m_manualTOD = 0.5f;
    float m_renderDistance = 256.0f;
    glm::vec4 m_skyColor = {0, 0, 0, 1};
};