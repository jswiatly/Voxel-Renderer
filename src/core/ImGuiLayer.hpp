#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

class VulkanContext;
class Window;
class Camera;

class ImGuiLayer {
    public:
        void init(VulkanContext& ctx, Window& window, VkRenderPass renderPass);
        void cleanup();

        void newFrame();
        void draw(const Camera& camera, float& timeOfDay, bool& manualTime, float& manualTOD, const glm::vec4& skyColor);
        void render();
        void renderDrawData(VkCommandBuffer commandBuffer);

    private:
        void createDescriptorPool();

        VulkanContext* m_ctx = nullptr;
        VkDescriptorPool m_pool = VK_NULL_HANDLE;
};