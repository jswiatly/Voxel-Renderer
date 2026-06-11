#pragma once

#include <vulkan/vulkan.h>
#include <mutex>
#include <string>
#include <vector>

class ValidationLogger {
    public:

    private:
        struct Entry {
        VkDebugUtilsMessageSeverityFlagBitsEXT severity;
        std::string message;
        };

        VkDebugUtilsMessengerEXT m_messenger = VK_NULL_HANDLE;
        std::vector<Entry> m_logs;
        std::mutex m_mutex
}