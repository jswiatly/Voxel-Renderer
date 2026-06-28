#include "core/Mesh.hpp"
#include "core/VulkanContext.hpp"
#include "core/Pipeline.hpp"
#include "core/Texture.hpp"
#include "core/Constants.hpp"

#include <array>
#include <cstring>
#include <stdexcept>

void Mesh::init(VulkanContext& ctx, Pipeline& pipeline, Texture& texture, const std::vector<Vertex>& vertices,
                const std::vector<uint32_t>& indices) {
    m_ctx = &ctx;
    m_indexCount = static_cast<uint32_t>(indices.size());
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    createVertexBuffer(vertices);
    createIndexBuffer(indices);
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets(pipeline, texture);
}

void Mesh::cleanup() {
    VkDevice device = m_ctx->device();
    VmaAllocator allocator = m_ctx->allocator();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vmaDestroyBuffer(allocator, m_uniformBuffers[i], m_uniformAllocations[i]);
    }
    vkDestroyDescriptorPool(device, m_descriptorPool, nullptr); // zwalnia też descriptor sety
    vmaDestroyBuffer(allocator, m_indexBuffer, m_indexAllocation);
    vmaDestroyBuffer(allocator, m_vertexBuffer, m_vertexAllocation);
}

void Mesh::createVertexBuffer(const std::vector<Vertex>& vertices) {
    VmaAllocator allocator = m_ctx->allocator();
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAllocation;
    m_ctx->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO, stagingBuffer,
                        stagingBufferAllocation, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    void* data;
    vmaMapMemory(allocator, stagingBufferAllocation, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vmaUnmapMemory(allocator, stagingBufferAllocation);

    m_ctx->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                        VMA_MEMORY_USAGE_AUTO, m_vertexBuffer, m_vertexAllocation);

    copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);
    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAllocation);
}

void Mesh::createIndexBuffer(const std::vector<uint32_t>& indices) {
    VmaAllocator allocator = m_ctx->allocator();
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingBufferAllocation;
    m_ctx->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO, stagingBuffer,
                        stagingBufferAllocation, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    void* data;
    vmaMapMemory(allocator, stagingBufferAllocation, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vmaUnmapMemory(allocator, stagingBufferAllocation);

    m_ctx->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                        VMA_MEMORY_USAGE_AUTO, m_indexBuffer, m_indexAllocation);

    copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);
    vmaDestroyBuffer(allocator, stagingBuffer, stagingBufferAllocation);
}

void Mesh::createUniformBuffers() {
    VmaAllocator allocator = m_ctx->allocator();
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniformAllocations.resize(MAX_FRAMES_IN_FLIGHT);
    m_uniformMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VmaAllocationCreateFlags flags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        m_ctx->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_AUTO, m_uniformBuffers[i],
                            m_uniformAllocations[i], flags);
        VmaAllocationInfo info;
        vmaGetAllocationInfo(allocator, m_uniformAllocations[i], &info);
        m_uniformMapped[i] = info.pMappedData;
    }
}

void Mesh::createDescriptorPool() {
    VkDevice device = m_ctx->device();
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

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void Mesh::createDescriptorSets(Pipeline& pipeline, Texture& texture) {
    VkDevice device = m_ctx->device();
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, pipeline.descriptorSetLayout());
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture.imageView();
        imageInfo.sampler = texture.sampler();

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
                               nullptr);
    }
}

void Mesh::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = m_ctx->beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    m_ctx->endSingleTimeCommands(commandBuffer);
}

void Mesh::updateUniforms(uint32_t frameIndex, const UniformBufferObject& ubo) {
    memcpy(m_uniformMapped[frameIndex], &ubo, sizeof(ubo));
}