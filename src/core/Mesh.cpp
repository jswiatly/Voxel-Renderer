#include "core/Mesh.hpp"
#include "core/VulkanContext.hpp"

#include <cstring>

void Mesh::init(VulkanContext& ctx, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    m_ctx = &ctx;
    m_indexCount = static_cast<uint32_t>(indices.size());
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    createVertexBuffer(vertices);
    createIndexBuffer(indices);
}

void Mesh::cleanup() {
    VmaAllocator allocator = m_ctx->allocator();
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

void Mesh::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = m_ctx->beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    m_ctx->endSingleTimeCommands(commandBuffer);
}