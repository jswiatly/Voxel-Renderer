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

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

void SetupVestaStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- 1. Sizing and Spacing ---
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ScrollbarSize = 15.0f;
    style.GrabMinSize = 10.0f;

    // --- 2. Borders & Rounding ---
    style.WindowRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    // --- 3. Color Palette: deep crystal sea ---

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.88f, 0.96f, 0.97f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.54f, 0.58f, 1.00f);

    // Backgrounds
    colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.09f, 0.11f, 1.00f); // Deep ocean
    colors[ImGuiCol_ChildBg] = ImVec4(0.05f, 0.11f, 0.14f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.04f, 0.09f, 0.11f, 0.96f);

    // Borders
    colors[ImGuiCol_Border] = ImVec4(0.10f, 0.32f, 0.38f, 0.70f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frames (Inputs, Checkboxes, etc.)
    colors[ImGuiCol_FrameBg] = ImVec4(0.07f, 0.17f, 0.21f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.10f, 0.26f, 0.32f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.13f, 0.36f, 0.44f, 1.00f);

    // Title Bars
    colors[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.24f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.03f, 0.07f, 0.09f, 1.00f);

    // Menus
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.06f, 0.14f, 0.17f, 1.00f);

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.03f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.12f, 0.32f, 0.38f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.16f, 0.44f, 0.52f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.20f, 0.58f, 0.68f, 1.00f);

    // Interactables
    colors[ImGuiCol_CheckMark] = ImVec4(0.30f, 0.95f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.20f, 0.75f, 0.85f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.95f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.09f, 0.30f, 0.38f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.13f, 0.44f, 0.55f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.18f, 0.60f, 0.72f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.09f, 0.30f, 0.38f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.13f, 0.44f, 0.55f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.18f, 0.60f, 0.72f, 1.00f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.06f, 0.18f, 0.23f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.15f, 0.48f, 0.58f, 1.00f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.10f, 0.32f, 0.40f, 1.00f);
    colors[ImGuiCol_TabDimmed] = ImVec4(0.04f, 0.11f, 0.14f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.06f, 0.18f, 0.23f, 1.00f);

    // Tables
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.08f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.12f, 0.34f, 0.42f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.08f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.04f);

    // Misc
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.70f, 0.85f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.40f, 0.95f, 1.00f, 0.90f);
    colors[ImGuiCol_NavCursor] = ImVec4(0.30f, 0.90f, 1.00f, 1.00f);

#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.20f, 0.70f, 0.85f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.04f, 0.09f, 0.11f, 1.00f);
#endif
}

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
        initImGui();
    }

    void DrawImGui() {
        ImGui::Begin("Debug");
        int hh = static_cast<int>(m_timeOfDay * 24.0f);
        int mm = static_cast<int>(m_timeOfDay * 24.0f * 60.0f) % 60;
        int ss = static_cast<int>(m_timeOfDay * 24.0f * 3600.0f) % 60;
        ImGui::Text("In-game time: %02d:%02d:%02d", hh, mm, ss);
        ImGui::ColorButton("Sky", ImVec4(m_skyColor.r, m_skyColor.g, m_skyColor.b, m_skyColor.a), 0, ImVec2(80, 20));
        ImGui::Checkbox("Manual time of day", &m_manualTime);
        if (m_manualTime) {
            ImGui::SliderFloat("Time of day", &m_manualTOD, 0.0f, 1.0f);
            m_timeOfDay = m_manualTOD;
        }
        ImGui::End();

        ImGui::Begin("Performance");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Begin("3D Orientation", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();

        float size = 120.0f;
        float axis_length = 45.0f;
        ImVec2 center = ImVec2(p.x + size / 2.0f, p.y + size / 2.0f);

        draw_list->AddCircleFilled(center, axis_length + 15.0f, IM_COL32(20, 20, 20, 150));
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::rotate(view, glm::radians(camera.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians(camera.yaw), glm::vec3(0.0f, 1.0f, 0.0f));

        struct Axis {
            glm::vec3 world_dir;
            ImU32 color;
            const char* label;
            glm::vec2 screen_pos;
            float depth;
        };

        Axis axes[3] = {{glm::vec3(1.0f, 0.0f, 0.0f), IM_COL32(255, 50, 50, 255), "X"},
                        {glm::vec3(0.0f, 1.0f, 0.0f), IM_COL32(50, 255, 50, 255), "Y"},
                        {glm::vec3(0.0f, 0.0f, 1.0f), IM_COL32(50, 100, 255, 255), "Z"}};

        for (int i = 0; i < 3; i++) {
            glm::vec4 view_dir = view * glm::vec4(axes[i].world_dir, 0.0f);

            axes[i].screen_pos = glm::vec2(center.x + view_dir.x * axis_length, center.y - view_dir.y * axis_length);
            axes[i].depth = view_dir.z;
        }

        std::sort(std::begin(axes), std::end(axes), [](const Axis& a, const Axis& b) { return a.depth < b.depth; });
        for (int i = 0; i < 3; i++) {
            ImVec2 end_pos = ImVec2(axes[i].screen_pos.x, axes[i].screen_pos.y);
            float thickness = (axes[i].depth > 0.0f) ? 3.5f : 1.5f;
            draw_list->AddLine(center, end_pos, axes[i].color, thickness);
            ImVec2 text_size = ImGui::CalcTextSize(axes[i].label);
            glm::vec2 dir_norm = glm::normalize(axes[i].screen_pos - glm::vec2(center.x, center.y));
            ImVec2 text_pos = ImVec2(end_pos.x + dir_norm.x * 12.0f - text_size.x / 2.0f,
                                     end_pos.y + dir_norm.y * 12.0f - text_size.y / 2.0f);
            ImU32 text_color = axes[i].color;
            if (axes[i].depth < 0.0f) {
                text_color = (text_color & 0x00FFFFFF) | 0x80000000;
            }
            draw_list->AddText(text_pos, text_color, axes[i].label);
        }
        draw_list->AddCircleFilled(center, 3.0f, IM_COL32(255, 255, 255, 255));
        ImGui::Dummy(ImVec2(size, size));
        ImGui::End();
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

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            DrawImGui();
            m_validationLog.drawImGuiWindow();
            ImGui::Render();
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

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(device, imguiPool, nullptr);

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

    void createImGuiDescriptorPool() {
        VkDevice device = m_context.device();
        std::array<VkDescriptorPoolSize, 1> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[0].descriptorCount = 1000;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &imguiPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create imgui descriptor pool!");
        }
    }

    void initImGui() {
        VkInstance instance = m_context.instance();
        VkPhysicalDevice physicalDevice = m_context.physicalDevice();
        VkDevice device = m_context.device();
        VkQueue graphicsQueue = m_context.graphicsQueue();
        createImGuiDescriptorPool();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        ImFontConfig cfg;
        cfg.OversampleH = 1;
        cfg.OversampleV = 1;
        cfg.PixelSnapH = true;
        // io.Fonts->AddFontFromFileTTF("fonts/VT323-Regular.ttf", 16.0f, &cfg);
        SetupVestaStyle();

        ImGui_ImplGlfw_InitForVulkan(window_.handle(), true);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = instance;
        init_info.PhysicalDevice = physicalDevice;
        init_info.Device = device;
        init_info.QueueFamily = m_context.findQueueFamilies(physicalDevice).graphicsFamily.value();
        init_info.Queue = graphicsQueue;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = imguiPool;
        init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
        init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;

        init_info.PipelineInfoMain.RenderPass = m_pipeline.renderPass();
        init_info.PipelineInfoMain.Subpass = 0;
        init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = nullptr;

        ImGui_ImplVulkan_Init(&init_info);
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

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

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