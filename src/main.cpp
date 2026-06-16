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

const std::string TEXTURE_PATH = "textures/test.jpg";

const int MAX_FRAMES_IN_FLIGHT = 2;

constexpr int WIDTH = 800;
constexpr int HEIGHT = 800;

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
        initInput();
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
    float clearColor[4] = {0, 0, 0, 1};
    float m_timeOfDay = 0.0f;
    bool m_manualTime = false;
    float m_manualTOD = 0.5f;

    glm::vec4 m_skyColor = {0, 0, 0, 1};

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    VkBuffer vertexBuffer;
    VmaAllocation vertexBufferAllocation;
    VkBuffer indexBuffer;
    VmaAllocation indexBufferAllocation;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VmaAllocation> uniformBuffersAllocation;
    std::vector<void*> uniformBuffersMapped;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorPool imguiPool;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    float lastX = 400, lastY = 300;
    bool firstMouse = true;
    bool cursorMode = false;
    bool MouseModeKeyWasPressed = false;

    void initInput() {
        glfwSetInputMode(window_.handle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

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
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        m_swapchain.createFramebuffers(renderPass);
        CreateRenderFinishedSemaphores();
        m_texture.init(m_context, TEXTURE_PATH);
        DumpVMAMemoryStats(m_context.allocator(), "vma_stats_init.json");
        generateTerrain(vertices, indices);
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();
        initImGui();
    }

    void processInput(GLFWwindow* window) {
        const float dt = time.getDeltaTime();

        bool fKeyPressed = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
        if (fKeyPressed && !MouseModeKeyWasPressed) {
            cursorMode = !cursorMode;
            glfwSetInputMode(window, GLFW_CURSOR, cursorMode ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
            if (!cursorMode)
                firstMouse = true;
        }
        MouseModeKeyWasPressed = fKeyPressed;

        if (!cursorMode) {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                camera.processKeyboard(0, dt);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                camera.processKeyboard(1, dt);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                camera.processKeyboard(2, dt);
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                camera.processKeyboard(3, dt);

            if (!ImGui::GetIO().WantCaptureMouse) {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                if (firstMouse) {
                    lastX = static_cast<float>(xpos);
                    lastY = static_cast<float>(ypos);
                    firstMouse = false;
                }
                float xoffset = static_cast<float>(xpos) - lastX;
                float yoffset = lastY - static_cast<float>(ypos);
                lastX = static_cast<float>(xpos);
                lastY = static_cast<float>(ypos);
                camera.processMouseMovement(xoffset, yoffset);
            }
        }
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
            processInput(window_.handle());
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

        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vmaDestroyBuffer(allocator, uniformBuffers[i], uniformBuffersAllocation[i]);
        }

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        m_texture.cleanup();

        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

        vmaDestroyBuffer(allocator, indexBuffer, indexBufferAllocation);

        vmaDestroyBuffer(allocator, vertexBuffer, vertexBufferAllocation);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        m_validationLog.cleanup(m_context.instance());
        m_context.cleanup();
    }

    void recreateSwapChain() {
        m_swapchain.recreate(window_, renderPass);
        CreateRenderFinishedSemaphores();
        for (auto semaphore : renderFinishedSemaphores) {
            vkDestroySemaphore(m_context.device(), semaphore, nullptr);
        }
        CreateRenderFinishedSemaphores();
    }

    void createRenderPass() {
        VkDevice device = m_context.device();

        VkAttachmentDescription colorAttachment{.format = m_swapchain.imageFormat(),
                                                .samples = VK_SAMPLE_COUNT_1_BIT,
                                                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

        VkAttachmentDescription depthAttachment{.format = m_swapchain.depthFormat(),
                                                .samples = VK_SAMPLE_COUNT_1_BIT,
                                                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        VkAttachmentReference colorAttachmentRef{.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        VkAttachmentReference depthAttachmentRef{.attachment = 1,
                                                 .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        VkSubpassDescription subpass{.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                                     .colorAttachmentCount = 1,
                                     .pColorAttachments = &colorAttachmentRef,
                                     .pDepthStencilAttachment = &depthAttachmentRef};

        VkSubpassDependency dependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo{.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                                              .attachmentCount = static_cast<uint32_t>(attachments.size()),
                                              .pAttachments = attachments.data(),
                                              .subpassCount = 1,
                                              .pSubpasses = &subpass,
                                              .dependencyCount = 1,
                                              .pDependencies = &dependency};

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void createDescriptorSetLayout() {
        VkDevice device = m_context.device();
        VkDescriptorSetLayoutBinding uboLayoutBinding{.binding = 0,
                                                      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                      .descriptorCount = 1,
                                                      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                                                      .pImmutableSamplers = nullptr};

        VkDescriptorSetLayoutBinding samplerLayoutBinding{.binding = 1,
                                                          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                          .descriptorCount = 1,
                                                          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                                                          .pImmutableSamplers = nullptr};

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void createGraphicsPipeline() {
        VkDevice device = m_context.device();
        auto vertShaderCode = readFile("shaders/vert.spv");
        auto fragShaderCode = readFile("shaders/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{.sType =
                                                                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                            .stage = VK_SHADER_STAGE_VERTEX_BIT,
                                                            .module = vertShaderModule,
                                                            .pName = "main"};

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{.sType =
                                                                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                                            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                                                            .module = fragShaderModule,
                                                            .pName = "main"};

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE};

        VkPipelineViewportStateCreateInfo viewportState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1};

        VkPipelineRasterizationStateCreateInfo rasterizer{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.0f};

        VkPipelineMultisampleStateCreateInfo multisampling{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
        };

        VkPipelineDepthStencilStateCreateInfo depthStencil{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE};

        VkPipelineColorBlendAttachmentState colorBlendAttachment{
            .blendEnable = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                              VK_COLOR_COMPONENT_A_BIT};

        VkPipelineColorBlendStateCreateInfo colorBlending{.sType =
                                                              VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                                          .logicOpEnable = VK_FALSE,
                                                          .logicOp = VK_LOGIC_OP_COPY,
                                                          .attachmentCount = 1,
                                                          .pAttachments = &colorBlendAttachment,
                                                          .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}};

        std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(glm::mat4);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    void createVertexBuffer() {
        VmaAllocator allocator = m_context.allocator();
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;
        m_context.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO, stagingBuffer,
                     stagingBufferAllocation, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        void* data;
        vmaMapMemory(allocator, stagingBufferAllocation, &data);
        memcpy(data, vertices.data(), (size_t)bufferSize);

        vmaUnmapMemory(allocator, stagingBufferAllocation);

        m_context.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VMA_MEMORY_USAGE_AUTO, vertexBuffer, vertexBufferAllocation);

        copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
        vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAllocation);
    }

    void createIndexBuffer() {
        VmaAllocator allocator = m_context.allocator();
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;
        m_context.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO, stagingBuffer,
                     stagingBufferAllocation, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        void* data;
        vmaMapMemory(allocator, stagingBufferAllocation, &data);
        memcpy(data, indices.data(), (size_t)bufferSize);
        vmaUnmapMemory(allocator, stagingBufferAllocation);

        m_context.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VMA_MEMORY_USAGE_AUTO, indexBuffer, indexBufferAllocation);

        copyBuffer(stagingBuffer, indexBuffer, bufferSize);
        vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAllocation);
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

    void createUniformBuffers() {
        VmaAllocator allocator = m_context.allocator();
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersAllocation.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VmaAllocationCreateFlags flags =
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
            m_context.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, uniformBuffers[i],
                         uniformBuffersAllocation[i], flags);
            VmaAllocationInfo info;
            vmaGetAllocationInfo(allocator, uniformBuffersAllocation[i], &info);
            uniformBuffersMapped[i] = info.pMappedData;
        }
    }

    void createDescriptorPool() {
        VkDevice device = m_context.device();
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
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

        init_info.PipelineInfoMain.RenderPass = renderPass;
        init_info.PipelineInfoMain.Subpass = 0;
        init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = nullptr;

        ImGui_ImplVulkan_Init(&init_info);
    }

    void createDescriptorSets() {
        VkDevice device = m_context.device();
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_texture.imageView();
            imageInfo.sampler = m_texture.sampler();

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                                   nullptr);
        }
    }

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = m_context.beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        m_context.endSingleTimeCommands(commandBuffer);
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
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = m_swapchain.framebuffer(imageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_swapchain.extent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{clearColor[0], clearColor[1], clearColor[2], clearColor[3]}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

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

        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                                &descriptorSets[currentFrame], 0, nullptr);

        glm::mat4 model = glm::mat4(1.0f);

        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

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
        UniformBufferObject ubo{};

        ubo.view = camera.getViewMatrix();
        ubo.proj =
            glm::perspective(glm::radians(45.0f), m_swapchain.extent().width / (float)m_swapchain.extent().height, 0.1f, 1000.0f);

        ubo.proj[1][1] *= -1;

        memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
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

    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkDevice device = m_context.device();
        VkShaderModuleCreateInfo createInfo{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                            .codeSize = code.size(),
                                            .pCode = reinterpret_cast<const uint32_t*>(code.data())};

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
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