#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <cstdint>
#include <string>

class VulkanContext;

class Texture {
  public:
    void init(VulkanContext& ctx, const std::string& path);
    void cleanup();

    VkImageView imageView() const { return m_imageView; }
    VkSampler sampler() const { return m_sampler; }

  private:
    void loadFromFile(const std::string& path);
    void generateMipmaps(VkFormat imageFormat, int32_t texWidth, int32_t texHeight);
    void createImageView();
    void createSampler();

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                               uint32_t mipLevels);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    VulkanContext* m_ctx = nullptr;

    VkImage m_image = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
    uint32_t m_mipLevels = 1;
};