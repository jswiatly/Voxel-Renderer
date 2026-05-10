#pragma once

#include <vector>
#include <optional>

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

class VulkanDevice {
    public:
        VulkanDevice();
        ~VulkanDevice();
    private:
        VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };;
	    VkDevice logicalDevice{ VK_NULL_HANDLE };
	    VkPhysicalDeviceProperties properties{};
	    VkPhysicalDeviceFeatures features{};
	    VkPhysicalDeviceFeatures enabledFeatures{};
	    VkPhysicalDeviceMemoryProperties memoryProperties{};
	    std::vector<VkQueueFamilyProperties> queueFamilyProperties{};
	    std::vector<std::string> supportedExtensions{};
	    VkCommandPool commandPool{ VK_NULL_HANDLE };
};