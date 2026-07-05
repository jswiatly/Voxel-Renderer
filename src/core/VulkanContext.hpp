#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <optional>
#include <vector>

class Window;

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanContext {
  public:
    void init(Window& window, bool enableValidationLayers, const VkDebugUtilsMessengerCreateInfoEXT* debugCreateInfo);
    void cleanup();

    VkInstance instance() const {
        return m_instance;
    }
    VkSurfaceKHR surface() const {
        return m_surface;
    }
    VkPhysicalDevice physicalDevice() const {
        return m_physicalDevice;
    }
    VkDevice device() const {
        return m_device;
    }
    VkQueue graphicsQueue() const {
        return m_graphicsQueue;
    }
    VkQueue presentQueue() const {
        return m_presentQueue;
    }
    VmaAllocator allocator() const {
        return m_allocator;
    }
    VkCommandPool commandPool() const {
        return m_commandPool;
    }

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage vmaUsage, VkBuffer& buffer,
                      VmaAllocation& allocation, VmaAllocationCreateFlags vmaFlags = 0);
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arrayLayers, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage vmaUsage, VkImage& image,
                     VmaAllocation& allocation);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels,
                                VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, uint32_t layerCount = 1);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

  private:
    void createInstance(bool enableValidationLayers, const VkDebugUtilsMessengerCreateInfoEXT* debugCreateInfo);
    void pickPhysicalDevice();
    void createLogicalDevice(bool enableValidationLayers);
    void createCommandPool();

    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);
    bool checkValidationLayerSupport();

    VmaAllocator m_allocator = VK_NULL_HANDLE;
    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
};