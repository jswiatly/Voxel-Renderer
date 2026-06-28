#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <vector>

class VulkanContext;
class Window;

class Swapchain {
  public:
    void init(VulkanContext& ctx, Window& window);

    void createFramebuffers(VkRenderPass renderPass);

    void recreate(Window& window, VkRenderPass renderPass);

    void cleanup();

    VkSwapchainKHR handle() const {
        return m_swapChain;
    }
    VkFormat imageFormat() const {
        return m_imageFormat;
    }
    VkFormat depthFormat() const {
        return m_depthFormat;
    }
    VkExtent2D extent() const {
        return m_extent;
    }
    VkFramebuffer framebuffer(uint32_t i) const {
        return m_framebuffers[i];
    }
    uint32_t imageCount() const {
        return static_cast<uint32_t>(m_images.size());
    }

  private:
    void createSwapChain(Window& window);
    void createImageViews();
    void createDepthResources();

    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available);
    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& available);
    VkExtent2D chooseExtent(Window& window, const VkSurfaceCapabilitiesKHR& capabilities);
    VkFormat findDepthFormat();
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features);

    VulkanContext* m_ctx = nullptr;

    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> m_images;
    VkFormat m_imageFormat;
    VkExtent2D m_extent;
    std::vector<VkImageView> m_imageViews;
    std::vector<VkFramebuffer> m_framebuffers;

    VkImage m_depthImage = VK_NULL_HANDLE;
    VmaAllocation m_depthAllocation = VK_NULL_HANDLE;
    VkImageView m_depthImageView = VK_NULL_HANDLE;
    VkFormat m_depthFormat;
};