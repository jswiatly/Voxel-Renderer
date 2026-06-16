#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

class VulkanContext;

class Pipeline {
  public:
    void init(VulkanContext& ctx, VkFormat colorFormat, VkFormat depthFormat);
    void cleanup();

    VkRenderPass renderPass() const { return m_renderPass; }
    VkDescriptorSetLayout descriptorSetLayout() const { return m_descriptorSetLayout; }
    VkPipelineLayout pipelineLayout() const { return m_pipelineLayout; }
    VkPipeline graphicsPipeline() const { return m_graphicsPipeline; }

  private:
    void createRenderPass(VkFormat colorFormat, VkFormat depthFormat);
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    VkShaderModule createShaderModule(const std::vector<char>& code);
    static std::vector<char> readFile(const std::string& filename);

    VulkanContext* m_ctx = nullptr;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
};