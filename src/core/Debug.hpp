#pragma once

#include <vulkan/vulkan.h>

#include <mutex>
#include <string>
#include <vector>

class ValidationLogger {
  public:
    VkDebugUtilsMessengerCreateInfoEXT makeCreateInfo();

    void setup(VkInstance instance);

    void cleanup(VkInstance instance);

    void drawImGuiWindow();

  private:
    struct Entry {
        VkDebugUtilsMessageSeverityFlagBitsEXT severity;
        std::string message;
    };

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData);

    void append(VkDebugUtilsMessageSeverityFlagBitsEXT severity, const char* message);

    VkDebugUtilsMessengerEXT m_messenger = VK_NULL_HANDLE;
    std::vector<Entry> m_logs;
    std::mutex m_mutex;
};