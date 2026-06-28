#include "core/Debug.hpp"

#include <imgui.h>

#include <iostream>
#include <stdexcept>

namespace {
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}
} // namespace

VkDebugUtilsMessengerCreateInfoEXT ValidationLogger::makeCreateInfo() {
    return {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debugCallback,
            .pUserData = this};
}

void ValidationLogger::setup(VkInstance instance) {
    VkDebugUtilsMessengerCreateInfoEXT createInfo = makeCreateInfo();
    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &m_messenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void ValidationLogger::cleanup(VkInstance instance) {
    if (m_messenger != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(instance, m_messenger, nullptr);
        m_messenger = VK_NULL_HANDLE;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL
ValidationLogger::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT,
                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {

    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        if (pCallbackData->pMessageIdName == nullptr || std::string(pCallbackData->pMessageIdName) != "USER") {
            return VK_FALSE;
        }
    }

    auto self = static_cast<ValidationLogger*>(pUserData);
    if (self != nullptr) {
        self->append(messageSeverity, pCallbackData->pMessage);
    }

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

void ValidationLogger::append(VkDebugUtilsMessageSeverityFlagBitsEXT severity, const char* message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logs.push_back({severity, message});
}

void ValidationLogger::drawImGuiWindow() {
    ImGui::Begin("Vulkan Validation Layers");
    ImGui::Separator();
    ImGui::BeginChild("LogRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    std::lock_guard<std::mutex> lock(m_mutex);

    for (const auto& log : m_logs) {
        ImVec4 color;
        std::string displayText;
        if (log.severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
            displayText = "[ERROR] " + log.message;
        } else if (log.severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            color = ImVec4(1.0f, 0.8f, 0.3f, 1.0f);
            displayText = "[WARNING] " + log.message;
        } else if (log.severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            color = ImVec4(0.4f, 0.8f, 0.3f, 1.0f);
            displayText = "[INFO] " + log.message;
        } else {
            color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
            displayText = "[OTHER] " + log.message;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextWrapped("%s", displayText.c_str());
        ImGui::PopStyleColor();
        ImGui::Separator();
    }
    ImGui::EndChild();
    ImGui::End();
}
