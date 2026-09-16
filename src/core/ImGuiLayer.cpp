#include "core/ImGuiLayer.hpp"
#include "core/VulkanContext.hpp"
#include "core/Window.hpp"
#include "core/Constants.hpp"
#include "scene/Camera.hpp"
#include "scene/Terrain.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <array>
#include <cmath>

namespace {

std::string groupThousands(uint32_t value) {
    std::string s = std::to_string(value);
    for (int i = static_cast<int>(s.size()) - 3; i > 0; i -= 3)
        s.insert(i, " ");
    return s;
}

void SetupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ScrollbarSize = 15.0f;
    style.GrabMinSize = 10.0f;

    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    colors[ImGuiCol_Text] = ImVec4(0.88f, 0.96f, 0.97f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.54f, 0.58f, 1.00f);

    colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.09f, 0.11f, 1.00f); // Deep ocean
    colors[ImGuiCol_ChildBg] = ImVec4(0.05f, 0.11f, 0.14f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.04f, 0.09f, 0.11f, 0.96f);

    colors[ImGuiCol_Border] = ImVec4(0.10f, 0.32f, 0.38f, 0.70f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    colors[ImGuiCol_FrameBg] = ImVec4(0.07f, 0.17f, 0.21f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.14f, 0.34f, 0.42f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.46f, 0.56f, 1.00f);

    colors[ImGuiCol_TitleBg] = ImVec4(0.05f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.24f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.03f, 0.07f, 0.09f, 1.00f);

    colors[ImGuiCol_MenuBarBg] = ImVec4(0.06f, 0.14f, 0.17f, 1.00f);

    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.03f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.12f, 0.32f, 0.38f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.16f, 0.44f, 0.52f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.20f, 0.58f, 0.68f, 1.00f);

    colors[ImGuiCol_CheckMark] = ImVec4(0.30f, 0.95f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.20f, 0.75f, 0.85f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.95f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.09f, 0.30f, 0.38f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.52f, 0.64f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.18f, 0.60f, 0.72f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.09f, 0.30f, 0.38f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.52f, 0.64f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.18f, 0.60f, 0.72f, 1.00f);

    colors[ImGuiCol_Tab] = ImVec4(0.06f, 0.18f, 0.23f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.15f, 0.48f, 0.58f, 1.00f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.10f, 0.32f, 0.40f, 1.00f);
    colors[ImGuiCol_TabDimmed] = ImVec4(0.04f, 0.11f, 0.14f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.06f, 0.18f, 0.23f, 1.00f);

    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.08f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.12f, 0.34f, 0.42f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.08f, 0.22f, 0.28f, 1.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.04f);

    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.70f, 0.85f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.40f, 0.95f, 1.00f, 0.90f);
    colors[ImGuiCol_NavCursor] = ImVec4(0.30f, 0.90f, 1.00f, 1.00f);

#ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview] = ImVec4(0.20f, 0.70f, 0.85f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.04f, 0.09f, 0.11f, 1.00f);
#endif
}

} // namespace

void ImGuiLayer::createDescriptorPool() {
    VkDevice device = m_ctx->device();
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSizes[0].descriptorCount = 1000;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSizes[1].descriptorCount = 1000;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create imgui descriptor pool!");
    }
}

void ImGuiLayer::init(VulkanContext& ctx, Window& window, VkRenderPass renderPass) {
    m_ctx = &ctx;
    createDescriptorPool();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    SetupStyle();

    ImGui_ImplGlfw_InitForVulkan(window.handle(), true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_ctx->instance();
    init_info.PhysicalDevice = m_ctx->physicalDevice();
    init_info.Device = m_ctx->device();
    init_info.QueueFamily = m_ctx->findQueueFamilies(m_ctx->physicalDevice()).graphicsFamily.value();
    init_info.Queue = m_ctx->graphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_pool;
    init_info.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
    init_info.PipelineInfoMain.RenderPass = renderPass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = nullptr;

    ImGui_ImplVulkan_Init(&init_info);
}

void ImGuiLayer::cleanup() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(m_ctx->device(), m_pool, nullptr);
}

void ImGuiLayer::newFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::render() {
    ImGui::Render();
}

void ImGuiLayer::renderDrawData(VkCommandBuffer commandBuffer) {
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void ImGuiLayer::draw(Camera& camera, float& timeOfDay, bool& manualTime, float& manualTOD, const glm::vec4& skyColor,
                      const RenderStats& stats, float& renderDistance, bool& fogEnabled, int& seed, int& worldSize,
                      bool& regenerate) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New World"))
                regenerate = true;
            ImGui::Separator();
            ImGui::MenuItem("Exit", nullptr, false, false);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scene")) {
            ImGui::MenuItem("Fog", nullptr, &fogEnabled);
            ImGui::MenuItem("Manual time of day", nullptr, &manualTime);
            ImGui::Separator();
            if (ImGui::MenuItem("Regenerate world"))
                regenerate = true;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Player HUD", nullptr, &m_showPlayerHud);
            ImGui::MenuItem("Crosshair", nullptr, &m_showCrosshair);
            ImGui::MenuItem("3D Orientation", nullptr, &m_showOrientationGizmo);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    const int chunksPerAxis = (worldSize + CHUNK_SIZE_X - 1) / CHUNK_SIZE_X;

    const int chunkX = static_cast<int>(std::floor((camera.position.x + worldSize * 0.5f) / CHUNK_SIZE_X));
    const int chunkZ = static_cast<int>(std::floor((camera.position.z + worldSize * 0.5f) / CHUNK_SIZE_Z));

    const bool insideWorld = chunkX >= 0 && chunkX < chunksPerAxis && chunkZ >= 0 && chunkZ < chunksPerAxis;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    if (m_showPlayerHud) {
        ImGui::SetNextWindowBgAlpha(0.55f);
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 16.0f, viewport->WorkPos.y + 52.0f),
                                ImGuiCond_Always, ImVec2(1.0f, 0.0f));

        ImGui::Begin("Player HUD", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs |
                         ImGuiWindowFlags_NoNav);

        ImGui::TextColored(camera.thirdPerson ? ImVec4(0.95f, 0.72f, 0.30f, 1.0f) : ImVec4(0.35f, 0.95f, 0.60f, 1.0f),
                           "%s", camera.thirdPerson ? "THIRD-PERSON" : "FIRST-PERSON");

        ImGui::Separator();
        ImGui::Text("X %+07.1f", camera.position.x);
        ImGui::Text("Y %+07.1f", camera.position.y);
        ImGui::Text("Z %+07.1f", camera.position.z);

        ImGui::Text("Yaw %6.1f", camera.yaw);
        ImGui::Text("Pitch %5.1f", camera.pitch);
        ImGui::Text("Speed %.1f", camera.movementSpeed);

        ImGui::Separator();
        if (insideWorld)
            ImGui::Text("Chunk [%d, %d]", chunkX, chunkZ);
        else
            ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.40f, 1.0f), "Outside world");

        ImGui::End();
    }

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    if (m_showCrosshair) {
        const ImVec2 crosshairCenter(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
                                     viewport->WorkPos.y + viewport->WorkSize.y * 0.5f);

        ImDrawList* draw = ImGui::GetForegroundDrawList();

        draw->AddLine({crosshairCenter.x - 8, crosshairCenter.y}, {crosshairCenter.x + 8, crosshairCenter.y},
                      IM_COL32_WHITE, 1.5f);

        draw->AddLine({crosshairCenter.x, crosshairCenter.y - 8}, {crosshairCenter.x, crosshairCenter.y + 8},
                      IM_COL32_WHITE, 1.5f);
    }

    // 1. Zwarty panel kontrolny z zakładkami
    ImGui::SetNextWindowSize(ImVec2(380, 450), ImGuiCond_FirstUseEver);
    ImGui::Begin("Control Panel");

    if (ImGui::BeginTabBar("MainTabs")) {

        // Zakładka Ustawień
        if (ImGui::BeginTabItem("Settings")) {
            if (ImGui::CollapsingHeader("Environment", ImGuiTreeNodeFlags_DefaultOpen)) {
                int hh = static_cast<int>(timeOfDay * 24.0f);
                int mm = static_cast<int>(timeOfDay * 24.0f * 60.0f) % 60;
                int ss = static_cast<int>(timeOfDay * 24.0f * 3600.0f) % 60;
                ImGui::Text("In-game time: %02d:%02d:%02d", hh, mm, ss);
                ImGui::ColorButton("Sky", ImVec4(skyColor.r, skyColor.g, skyColor.b, skyColor.a), 0, ImVec2(80, 20));
                ImGui::Checkbox("Manual time of day", &manualTime);
                if (manualTime) {
                    ImGui::SliderFloat("Time of day", &manualTOD, 0.0f, 1.0f);
                    timeOfDay = manualTOD;
                }
                ImGui::Checkbox("Fog", &fogEnabled);
            }

            if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::SliderFloat("Render distance", &renderDistance, 16.0f, MAX_RENDER_DISTANCE, "%.0f",
                                   ImGuiSliderFlags_Logarithmic);
                ImGui::SliderFloat("Camera speed", &camera.movementSpeed, 1.0f, 50.0f, "%.1f");
            }

            if (ImGui::CollapsingHeader("World Generation", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::PushItemWidth(100.0f);
                ImGui::InputInt("Seed", &seed);
                ImGui::SameLine();
                ImGui::InputInt("World size", &worldSize);
                ImGui::PopItemWidth();

                ImGui::SameLine();
                if (ImGui::Button("Regenerate"))
                    regenerate = true;
            }
            ImGui::EndTabItem();
        }

        float ms = ImGui::GetIO().DeltaTime * 1000.0f;
        m_frameTimes[m_frameOffset] = ms;
        m_frameOffset = (m_frameOffset + 1) % FRAME_HISTORY;

        // Zakładka Wydajności
        if (ImGui::BeginTabItem("Performance")) {

            ImGui::Text("Average: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);

            ImVec4 histColor = ImVec4(0.20f, 0.80f, 0.30f, 1.0f);
            if (ms > 16.6f)
                histColor = ImVec4(0.80f, 0.80f, 0.20f, 1.0f);
            if (ms > 33.3f)
                histColor = ImVec4(0.90f, 0.20f, 0.20f, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, histColor);
            ImGui::PlotHistogram("##frametime", m_frameTimes, FRAME_HISTORY, m_frameOffset, nullptr, 0.0f, FLT_MAX,
                                 ImVec2(0, 50));
            ImGui::PopStyleColor();

            ImGui::Text("CPU Time: %.3f ms", stats.cpuTimeMs);
            ImGui::Text("GPU Time: %.3f ms", stats.gpuTimeMs);
            ImGui::Spacing();

            ImVec4 frameColor = ImVec4(0.20f, 0.80f, 0.30f, 1.0f);
            if (ms > 16.6f)
                frameColor = ImVec4(0.80f, 0.80f, 0.20f, 1.0f);
            if (ms > 33.3f)
                frameColor = ImVec4(0.90f, 0.20f, 0.20f, 1.0f);

            ImGui::TextColored(frameColor, "Frame: %.2f ms", ms);
            ImGui::SameLine();
            ImGui::TextDisabled("(target: 16.67 ms)");

            ImGui::TextDisabled("Last submitted scene");

            const auto drawChunkBar = [](const char* label, uint32_t drawn, uint32_t total) {
                const float fraction = total == 0 ? 0.0f : static_cast<float>(drawn) / total;
                const std::string overlay = groupThousands(drawn) + " / " + groupThousands(total);

                ImGui::TextUnformatted(label);
                ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f), overlay.c_str());
            };

            drawChunkBar("Terrain chunks drawn", stats.terrainChunksDrawn, stats.terrainChunksTotal);
            drawChunkBar("Water chunks drawn", stats.waterChunksDrawn, stats.waterChunksTotal);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Scene draw calls: %s", groupThousands(stats.sceneDrawCalls).c_str());
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End(); // Koniec panelu głównego

    // 2. Nienaprzykrzający się HUD z orientacją 3D
    if (m_showOrientationGizmo) {
        ImGuiWindowFlags overlayFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
                                        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing |
                                        ImGuiWindowFlags_NoNav;

        ImGui::SetNextWindowBgAlpha(0.35f);
        ImGui::Begin("3D Orientation", nullptr, overlayFlags);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        float size = 120.0f;
        float axis_length = 45.0f;
        ImVec2 center(p.x + size / 2.0f, p.y + size / 2.0f);

        draw_list->AddCircleFilled(center, axis_length + 15.0f, IM_COL32(20, 20, 20, 150));

        glm::mat4 view(1.0f);
        view = glm::rotate(view, glm::radians(camera.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians(camera.yaw), glm::vec3(0.0f, 1.0f, 0.0f));

        struct Axis {
            glm::vec3 world_dir;
            ImU32 color;
            const char* label;
            glm::vec2 screen_pos;
            float depth;
        };

        Axis axes[3] = {
            {glm::vec3(1.0f, 0.0f, 0.0f), IM_COL32(255, 50, 50, 255), "X"},
            {glm::vec3(0.0f, 1.0f, 0.0f), IM_COL32(50, 255, 50, 255), "Y"},
            {glm::vec3(0.0f, 0.0f, 1.0f), IM_COL32(50, 100, 255, 255), "Z"},
        };

        for (Axis& axis : axes) {
            const glm::vec4 viewDir = view * glm::vec4(axis.world_dir, 0.0f);

            axis.screen_pos = glm::vec2(center.x + viewDir.x * axis_length, center.y - viewDir.y * axis_length);
            axis.depth = viewDir.z;
        }

        std::sort(std::begin(axes), std::end(axes), [](const Axis& a, const Axis& b) { return a.depth < b.depth; });

        for (const Axis& axis : axes) {
            const ImVec2 endPos(axis.screen_pos.x, axis.screen_pos.y);
            const float thickness = axis.depth > 0.0f ? 3.5f : 1.5f;

            draw_list->AddLine(center, endPos, axis.color, thickness);

            const ImVec2 textSize = ImGui::CalcTextSize(axis.label);
            const glm::vec2 direction = glm::normalize(axis.screen_pos - glm::vec2(center.x, center.y));

            const ImVec2 textPos(endPos.x + direction.x * 12.0f - textSize.x / 2.0f,
                                 endPos.y + direction.y * 12.0f - textSize.y / 2.0f);

            ImU32 textColor = axis.color;
            if (axis.depth < 0.0f)
                textColor = (textColor & 0x00FFFFFF) | 0x80000000;

            draw_list->AddText(ImVec2(textPos.x + 1.0f, textPos.y + 1.0f), IM_COL32(0, 0, 0, 255), axis.label);
            draw_list->AddText(textPos, textColor, axis.label);
        }

        draw_list->AddCircleFilled(center, 3.0f, IM_COL32_WHITE);
        ImGui::Dummy(ImVec2(size, size));
        ImGui::End();
    }
}