#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <vector>

class VulkanContext;
class Window;
class Swapchain;
class Pipeline;
class Mesh;
class ImGuiLayer;
class Skybox;
struct UniformBufferObject;

class Renderer {
  public:
    void init(VulkanContext& ctx, Window& window, Swapchain& swapchain, Pipeline& pipeline, std::vector<Mesh>& chunks,
              ImGuiLayer& imgui, Skybox& skybox);
    void cleanup();
    void drawFrame(const UniformBufferObject& ubo, const glm::vec4& clearColor);
    void setRenderDistance(float d) {
        m_renderDistance = d;
    }

  private:
    void createCommandBuffers();
    void createSyncObjects();
    void createRenderFinishedSemaphores();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const glm::vec4& clearColor);
    void recreateSwapchain();

    VulkanContext* m_ctx = nullptr;
    Window* m_window = nullptr;
    Swapchain* m_swapchain = nullptr;
    Pipeline* m_pipeline = nullptr;
    std::vector<Mesh>* m_chunks = nullptr;
    ImGuiLayer* m_imgui = nullptr;
    Skybox* m_skybox = nullptr;

    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    uint32_t m_currentFrame = 0;
    float m_renderDistance = 256.0f;
    glm::vec3 m_camPos{0.0f};
};