#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "core/Engine.hpp"
#include "renderer/Vertex.hpp"
#include "scene/Sky.hpp"
#include "scene/Terrain.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

void Engine::run() {
    m_input.init(window_.handle());
    initVulkan();
    mainLoop();
    cleanup();
}

void Engine::initVulkan() {
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    const VkDebugUtilsMessengerCreateInfoEXT* pDebugInfo = nullptr;
    if (enableValidationLayers) {
        debugCreateInfo = m_validationLog.makeCreateInfo();
        pDebugInfo = &debugCreateInfo;
    }
    m_context.init(window_, enableValidationLayers, pDebugInfo);
    if (enableValidationLayers) m_validationLog.setup(m_context.instance());

    m_swapchain.init(m_context, window_);
    m_pipeline.init(m_context, m_swapchain.imageFormat(), m_swapchain.depthFormat());
    m_swapchain.createFramebuffers(m_pipeline.renderPass());
    m_texture.init(m_context, TEXTURE_PATH);
    DumpVMAMemoryStats(m_context.allocator(), "vma_stats_init.json");

    std::vector<Chunk> chunks = generateChunkedTerrain(512);
    m_chunks.reserve(chunks.size());
    for (Chunk& c : chunks) {
        if (c.vertices.empty()) continue;
        m_chunks.emplace_back();
        m_chunks.back().init(m_context, m_pipeline, m_texture, c.vertices, c.indices);
        m_chunks.back().setCenter(c.center);
    }
    m_imgui.init(m_context, window_, m_pipeline.renderPass());
    m_renderer.init(m_context, window_, m_swapchain, m_pipeline, m_chunks, m_imgui);
}

void Engine::mainLoop() {
    while (!window_.shouldClose()) {
        glfwPollEvents();
        time.update();

        constexpr float DAY_LENGTH_GAME_SECONDS = 250.0f; // DEFAULT: 86400
        if (!m_manualTime) {
            m_timeOfDay = std::fmod(static_cast<float>(time.getGameTimeSeconds()), DAY_LENGTH_GAME_SECONDS) /
                          DAY_LENGTH_GAME_SECONDS;
        }
        m_skyColor = getSkyColor(m_timeOfDay);

        m_imgui.newFrame();
        uint32_t vtot = 0, itot = 0;
        for (Mesh& m : m_chunks) { vtot += m.vertexCount(); itot += m.indexCount(); }
        ImGuiLayer::RenderStats stats{ vtot, itot, static_cast<uint32_t>(m_chunks.size()) };
        m_imgui.draw(camera, m_timeOfDay, m_manualTime, m_manualTOD, m_skyColor, stats, m_renderDistance);
        m_validationLog.drawImGuiWindow();
        m_imgui.render();
        m_input.process(window_.handle(), camera, time.getDeltaTime());

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                          m_swapchain.extent().width / (float)m_swapchain.extent().height, 0.1f, 1000.0f);
        proj[1][1] *= -1;
        UniformBufferObject ubo{};
        ubo.view = view;
        ubo.proj = proj;
        ubo.sunDir = glm::vec4(getSunDirection(m_timeOfDay), 0.0f);
        ubo.sunColor = glm::vec4(getSunColor(m_timeOfDay), 0.35f);
        m_renderer.setRenderDistance(m_renderDistance);
        m_renderer.drawFrame(ubo, m_skyColor);
    }

    vkDeviceWaitIdle(m_context.device());
}

void Engine::cleanup() {
    m_renderer.cleanup();
    m_swapchain.cleanup();
    m_imgui.cleanup();
    m_pipeline.cleanup();
    for (Mesh& m : m_chunks) m.cleanup();
    m_texture.cleanup();
    m_validationLog.cleanup(m_context.instance());
    m_context.cleanup();
}

void Engine::DumpVMAMemoryStats(VmaAllocator allocator, const char* filename) {
    char* statsString = nullptr;
    vmaBuildStatsString(allocator, &statsString, VK_TRUE);

    std::ofstream outFile(filename);
    if (outFile.is_open()) {
        outFile << statsString;
        outFile.close();
        std::cout << "VMA stats saved to: " << filename << std::endl;
    } else {
        std::cerr << "ERROR: Cant open save file!" << std::endl;
    }

    vmaFreeStatsString(allocator, statsString);
}