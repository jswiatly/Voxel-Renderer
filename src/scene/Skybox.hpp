#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vector>
#include "renderer/Vertex.hpp"

class VulkanContext;

class Skybox {
  public:
    void init(VulkanContext& ctx, VkRenderPass renderPass);
    void cleanup();
    void updateUniforms(uint32_t frame, const UniformBufferObject& ubo);
    void record(VkCommandBuffer cmd, uint32_t frame);

  private:
    void createDescriptorSetLayout();
    void createPipeline(VkRenderPass renderPass);
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();

    VulkanContext* m_ctx = nullptr;
    VkDescriptorSetLayout m_setLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptorSets;
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VmaAllocation> m_uniformAllocations;
    std::vector<void*> m_uniformMapped;
};