#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

#include "renderer/Vertex.hpp" // Vertex + UniformBufferObject

class VulkanContext;
class Pipeline;
class Texture;

class Mesh {
  public:
    void init(VulkanContext& ctx, Pipeline& pipeline, Texture& texture, const std::vector<Vertex>& vertices,
              const std::vector<uint32_t>& indices);
    void cleanup();

    void updateUniforms(uint32_t frameIndex, const UniformBufferObject& ubo);

    void setCenter(const glm::vec3& c) {
        m_center = c;
    }
    glm::vec3 center() const {
        return m_center;
    }

    VkBuffer vertexBuffer() const {
        return m_vertexBuffer;
    }
    VkBuffer indexBuffer() const {
        return m_indexBuffer;
    }
    uint32_t indexCount() const {
        return m_indexCount;
    }
    uint32_t vertexCount() const {
        return m_vertexCount;
    }
    VkDescriptorSet descriptorSet(uint32_t frameIndex) const {
        return m_descriptorSets[frameIndex];
    }

  private:
    void createVertexBuffer(const std::vector<Vertex>& vertices);
    void createIndexBuffer(const std::vector<uint32_t>& indices);
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets(Pipeline& pipeline, Texture& texture);
    void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

    VulkanContext* m_ctx = nullptr;

    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VmaAllocation m_vertexAllocation = VK_NULL_HANDLE;
    VkBuffer m_indexBuffer = VK_NULL_HANDLE;
    VmaAllocation m_indexAllocation = VK_NULL_HANDLE;
    uint32_t m_indexCount = 0;
    uint32_t m_vertexCount = 0;
    glm::vec3 m_center{0.0f};

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VmaAllocation> m_uniformAllocations;
    std::vector<void*> m_uniformMapped;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptorSets;
};