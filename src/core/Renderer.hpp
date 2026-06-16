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

class Renderer {
    public:
        void init(VulkanContext& ctx, Window& window, Swapchain& swapchain, Pipeline& pipeline, Mesh& mesh, ImGuiLayer& imgui);
        void cleanup();
        void drawFrame(const glm::mat4& view, const glm::mat4& proj, const glm::vec4& clearColor);

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
        Mesh* m_mesh = nullptr;
        ImGuiLayer* m_imgui = nullptr;

        std::vector<VkCommandBuffer> m_commandBuffers;
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
        uint32_t m_currentFrame = 0;
};