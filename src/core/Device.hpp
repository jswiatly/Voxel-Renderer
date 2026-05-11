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

struct VulkanDevice2
{
	// Physical device representation 
	VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };;
	// Logical device representation (application's view of the device) 
	VkDevice logicalDevice{ VK_NULL_HANDLE };
	// Properties of the physical device including limits that the application can check against 
	VkPhysicalDeviceProperties properties{};
	// Features of the physical device that an application can use to check if a feature is supported 
	VkPhysicalDeviceFeatures features{};
	// Features that have been enabled for use on the physical device 
	VkPhysicalDeviceFeatures enabledFeatures{};
	// Memory types and heaps of the physical device 
	VkPhysicalDeviceMemoryProperties memoryProperties{};
	// Queue family properties of the physical device 
	std::vector<VkQueueFamilyProperties> queueFamilyProperties{};
	// List of extensions supported by the device 
	std::vector<std::string> supportedExtensions{};
	// Default command pool for the graphics queue family index 
	VkCommandPool commandPool{ VK_NULL_HANDLE };
	// Contains queue family indices 
};


class VulkanDevice {
    public:
        //VulkanDevice(VkPhysicalDevice physicalDevice);
       // ~VulkanDevice();
    private:
        VkInstance instance;
        VkSurfaceKHR surface;
        std::vector<const char*> deviceExtensions;
        std::vector<const char*> validationLayers;
        bool enableValidationLayers;

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