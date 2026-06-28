#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

class VulkanContext;
class Window;
class Camera;

class ImGuiLayer {
  public:
    struct RenderStats {
        uint32_t vertices = 0;
        uint32_t indices = 0;
        uint32_t drawCalls = 0;
    };
    void init(VulkanContext& ctx, Window& window, VkRenderPass renderPass);
    void cleanup();

    void newFrame();
    void draw(Camera& camera, float& timeOfDay, bool& manualTime, float& manualTOD, const glm::vec4& skyColor,
              const RenderStats& stats, float& renderDistance, bool& fogEnabled);
    void render();
    void renderDrawData(VkCommandBuffer commandBuffer);

  private:
    void createDescriptorPool();

    VulkanContext* m_ctx = nullptr;
    VkDescriptorPool m_pool = VK_NULL_HANDLE;

    static constexpr int FRAME_HISTORY = 90;
    float m_frameTimes[FRAME_HISTORY] = {};
    int m_frameOffset = 0;
};