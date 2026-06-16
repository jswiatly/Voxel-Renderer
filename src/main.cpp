#define GLFW_INCLUDE_VULKAN
#define STB_IMAGE_IMPLEMENTATION

#include <GLFW/glfw3.h>
#include <glm/gtc/constants.hpp>
#include <stb_image.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <mutex>
#include <optional>
#include <set>
#include <stdexcept>
#include <unordered_map>

#include "Time.hpp"
#include "core/VulkanContext.hpp"
#include "core/Window.hpp"
#include "core/Debug.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "renderer/Vertex.hpp"
#include "scene/Camera.hpp"
#include "scene/Sky.hpp"
#include "scene/Terrain.hpp"
#include "core/Swapchain.hpp"
#include "core/Texture.hpp"
#include "core/InputHandler.hpp"
#include "core/Pipeline.hpp"
#include "core/Constants.hpp"
#include "core/Mesh.hpp"
#include "core/ImGuiLayer.hpp"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class Application {
  public:
    void run() {
        m_input.init(window_.handle());
        initVulkan();
        mainLoop();
        cleanup();
    }

  private:
    Window window_{WIDTH, HEIGHT, "Vesta"};
    Camera camera;
    ValidationLogger m_validationLog;
    vkr::Time time;
    VulkanContext m_context;
    Swapchain m_swapchain;
    Texture m_texture;
    InputHandler m_input;
    Pipeline m_pipeline;
    Mesh m_mesh;
    ImGuiLayer m_imgui;

    float clearColor[4] = {0, 0, 0, 1};
    float m_timeOfDay = 0.0f;
    bool m_manualTime = false;
    float m_manualTOD = 0.5f;

    glm::vec4 m_skyColor = {0, 0, 0, 1};

    VkDescriptorPool imguiPool;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    void initVulkan() {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        const VkDebugUtilsMessengerCreateInfoEXT* pDebugInfo = nullptr;
        if (enableValidationLayers) {
            debugCreateInfo = m_validationLog.makeCreateInfo();
            pDebugInfo = &debugCreateInfo;
        }
        m_context.init(window_, enableValidationLayers, pDebugInfo);
        if (enableValidationLayers) m_validationLog.setup(m_context.instance());

        m_swapchain.init(m_context, window_);
        m_pipeline.init(m_context, m_swapchain.imageFormat(), m_swapchain.depthFormat());
        m_swapchain.createFramebuffers(m_pipeline.renderPass());
        CreateRenderFinishedSemaphores();
        m_texture.init(m_context, TEXTURE_PATH);
        DumpVMAMemoryStats(m_context.allocator(), "vma_stats_init.json");
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        generateTerrain(vertices, indices);
        m_mesh.init(m_context, m_pipeline, m_texture, vertices, indices);
        createCommandBuffers();
        createSyncObjects();
        m_imgui.init(m_context, window_, m_pipeline.renderPass());
    }

    void mainLoop() {
        while (!window_.shouldClose()) {
            glfwPollEvents();
            time.update();

            constexpr float DAY_LENGTH_GAME_SECONDS = 250.0f; // DEFAULT: 86400
            if (!m_manualTime) {
                m_timeOfDay = std::fmod(static_cast<float>(time.getGameTimeSeconds()), DAY_LENGTH_GAME_SECONDS) /
                              DAY_LENGTH_GAME_SECONDS;
            }
            m_skyColor = getSkyColor(m_timeOfDay);
            clearColor[0] = m_skyColor.r;
            clearColor[1] = m_skyColor.g;
            clearColor[2] = m_skyColor.b;
            clearColor[3] = m_skyColor.a;

            m_imgui.newFrame();
            m_imgui.draw(camera, m_timeOfDay, m_manualTime, m_manualTOD, m_skyColor);
            m_validationLog.drawImGuiWindow();
            m_imgui.render();
            m_input.process(window_.handle(), camera, time.getDeltaTime());
            drawFrame();
        }

        vkDeviceWaitIdle(m_context.device());
    }

    void cleanup() {
        VkDevice device = m_context.device();
        VmaAllocator allocator = m_context.allocator();
        m_swapchain.cleanup();
        for (auto semaphore : renderFinishedSemaphores) {
            vkDestroySemaphore(device, semaphore, nullptr);
        }

        m_imgui.cleanup();
        m_pipeline.cleanup();
        m_mesh.cleanup();
        m_texture.cleanup();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        m_validationLog.cleanup(m_context.instance());
        m_context.cleanup();
    }

    void recreateSwapChain() {
        m_swapchain.recreate(window_, m_pipeline.renderPass());
        CreateRenderFinishedSemaphores();
        for (auto semaphore : renderFinishedSemaphores) {
            vkDestroySemaphore(m_context.device(), semaphore, nullptr);
        }
        CreateRenderFinishedSemaphores();
    } 

    void DumpVMAMemoryStats(VmaAllocator allocator, const char* filename) {
        char* statsString = nullptr;

        vmaBuildStatsString(allocator, &statsString, VK_TRUE);

        std::ofstream outFile(filename);
        if (outFile.is_open()) {
            outFile << statsString;
            outFile.close();
            std::cout << "VMA stats saved to: " << filename << std::endl;
        } else {
            std::cerr << "ERROR: Cant open save file!" << std::endl;
        }

        vmaFreeStatsString(allocator, statsString);
    }

    void createCommandBuffers() {
        VkDevice device = m_context.device();
        VkCommandPool commandPool = m_context.commandPool();
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_pipeline.renderPass();
        renderPassInfo.framebuffer = m_swapchain.framebuffer(imageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapchain.extent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{clearColor[0], clearColor[1], clearColor[2], clearColor[3]}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.graphicsPipeline());

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)m_swapchain.extent().width;
        viewport.height = (float)m_swapchain.extent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_swapchain.extent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = {m_mesh.vertexBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, m_mesh.indexBuffer(), 0, VK_INDEX_TYPE_UINT32);

        VkDescriptorSet descriptorSet = m_mesh.descriptorSet(currentFrame);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline.pipelineLayout(), 0, 1,
                                &descriptorSet, 0, nullptr);

        glm::mat4 model = glm::mat4(1.0f);

        vkCmdPushConstants(commandBuffer, m_pipeline.pipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_mesh.indexCount()), 1, 0, 0, 0);

        m_imgui.renderDrawData(commandBuffer);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void createSyncObjects() {
        VkDevice device = m_context.device();
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void CreateRenderFinishedSemaphores() {
        VkDevice device = m_context.device();
        renderFinishedSemaphores.resize(m_swapchain.imageCount());

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        for (size_t i = 0; i < m_swapchain.imageCount(); i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create render finished semaphores!");
            }
        }
    }

    void updateUniformBuffer(uint32_t currentImage) {
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 proj = glm::perspective(glm::radians(45.0f),
                                          m_swapchain.extent().width / (float)m_swapchain.extent().height, 0.1f, 1000.0f);
        proj[1][1] *= -1;
        m_mesh.updateUniforms(currentImage, view, proj);
    }

    void drawFrame() {
        VkDevice device = m_context.device();
        VkQueue graphicsQueue = m_context.graphicsQueue();
        VkQueue presentQueue = m_context.presentQueue();
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, m_swapchain.handle(), UINT64_MAX, imageAvailableSemaphores[currentFrame],
                                                VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        updateUniformBuffer(currentFrame);

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame],
                             /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[imageIndex]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {m_swapchain.handle()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window_.wasResized()) {
            window_.resetResizeFlag();
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
};

int main() {
    Application app;

    try {
        std::cout << R"(
__/\\\________/\\\__/\\\\\\\\\\\\\\\_____/\\\\\\\\\\\____/\\\\\\\\\\\\\\\_____/\\\\\\\\\____        
 _\/\\\_______\/\\\_\/\\\///////////____/\\\/////////\\\_\///////\\\/////____/\\\\\\\\\\\\\__       
  _\//\\\______/\\\__\/\\\______________\//\\\______\///________\/\\\________/\\\/////////\\\_      
   __\//\\\____/\\\___\/\\\\\\\\\\\_______\////\\\_______________\/\\\_______\/\\\_______\/\\\_     
    ___\//\\\__/\\\____\/\\\///////___________\////\\\____________\/\\\_______\/\\\\\\\\\\\\\\\_    
     ____\//\\\/\\\_____\/\\\_____________________\////\\\_________\/\\\_______\/\\\/////////\\\_   
      _____\//\\\\\______\/\\\______________/\\\______\//\\\________\/\\\_______\/\\\_______\/\\\_  
       ______\//\\\_______\/\\\\\\\\\\\\\\\_\///\\\\\\\\\\\/_________\/\\\_______\/\\\_______\/\\\_ 
        _______\///________\///////////////____\///////////___________\///________\///________\///__)"
                  << "\n\n";

        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}