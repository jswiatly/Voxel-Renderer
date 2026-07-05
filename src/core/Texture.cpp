#include "core/Texture.hpp"
#include "core/VulkanContext.hpp"

#include <stb_image.h> // tylko deklaracje — STB_IMAGE_IMPLEMENTATION zostaje w main.cpp

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

void Texture::init(VulkanContext& ctx, const std::vector<std::string>& paths) {
    m_ctx = &ctx;
    loadFromFiles(paths);
    createImageView();
    createSampler();
}

void Texture::cleanup() {
    VkDevice device = m_ctx->device();
    vkDestroySampler(device, m_sampler, nullptr);
    vkDestroyImageView(device, m_imageView, nullptr);
    vmaDestroyImage(m_ctx->allocator(), m_image, m_allocation);
}

void Texture::loadFromFiles(const std::vector<std::string>& paths) {
    VmaAllocator allocator = m_ctx->allocator();
    m_layerCount = static_cast<uint32_t>(paths.size());

    int texWidth = 0, texHeight = 0, texChannels;
    std::vector<stbi_uc*> layers;
    for (const std::string& p : paths) {
        int w, h;
        stbi_uc* pixels = stbi_load(p.c_str(), &w, &h, &texChannels, STBI_rgb_alpha);
        if (!pixels)
            throw std::runtime_error("failed to load texture image: " + p);
        if (!layers.empty() && (w != texWidth || h != texHeight))
            throw std::runtime_error("texture array layers must match in size: " + p);
        texWidth = w;
        texHeight = h;
        layers.push_back(pixels);
    }

    VkDeviceSize layerSize = VkDeviceSize(texWidth) * texHeight * 4;
    m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    m_ctx->createBuffer(layerSize * m_layerCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO,
                        stagingBuffer, stagingAllocation, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    void* data;
    vmaMapMemory(allocator, stagingAllocation, &data);
    for (size_t i = 0; i < layers.size(); ++i) {
        memcpy(static_cast<char*>(data) + i * layerSize, layers[i], layerSize);
        stbi_image_free(layers[i]);
    }
    vmaUnmapMemory(allocator, stagingAllocation);

    m_ctx->createImage(texWidth, texHeight, m_mipLevels, m_layerCount, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                       VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                       VMA_MEMORY_USAGE_AUTO, m_image, m_allocation);

    transitionImageLayout(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels);
    copyBufferToImage(stagingBuffer, m_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    generateMipmaps(VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight);

    vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
}

void Texture::generateMipmaps(VkFormat imageFormat, int32_t texWidth, int32_t texHeight) {
    VkPhysicalDevice physicalDevice = m_ctx->physicalDevice();
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = m_ctx->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                                 .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                 .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                 .image = m_image,
                                 .subresourceRange = {
                                     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                     .levelCount = 1,
                                     .baseArrayLayer = 0,
                                     .layerCount = m_layerCount,
                                 }};

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < m_mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                             nullptr, 0, nullptr, 1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = m_layerCount;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = m_layerCount;

        vkCmdBlitImage(commandBuffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                             nullptr, 0, nullptr, 1, &barrier);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = m_mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);

    m_ctx->endSingleTimeCommands(commandBuffer);
}

void Texture::createImageView() {
    m_imageView = m_ctx->createImageView(m_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels,
                                         VK_IMAGE_VIEW_TYPE_2D_ARRAY, m_layerCount);
}

void Texture::createSampler() {
    VkPhysicalDevice physicalDevice = m_ctx->physicalDevice();
    VkDevice device = m_ctx->device();
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                                    .magFilter = VK_FILTER_LINEAR,
                                    .minFilter = VK_FILTER_LINEAR,
                                    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                                    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                                    .anisotropyEnable = VK_TRUE,
                                    .maxAnisotropy = properties.limits.maxSamplerAnisotropy,
                                    .compareEnable = VK_FALSE,
                                    .compareOp = VK_COMPARE_OP_ALWAYS,
                                    .minLod = 0.0f,
                                    .maxLod = static_cast<float>(m_mipLevels),
                                    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                                    .unnormalizedCoordinates = VK_FALSE};

    if (vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void Texture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                                    uint32_t mipLevels) {
    VkCommandBuffer commandBuffer = m_ctx->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                             .baseMipLevel = 0,
                             .levelCount = mipLevels,
                             .baseArrayLayer = 0,
                             .layerCount = m_layerCount},
    };

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    m_ctx->endSingleTimeCommands(commandBuffer);
}

void Texture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = m_ctx->beginSingleTimeCommands();

    std::vector<VkBufferImageCopy> regions(m_layerCount);
    VkDeviceSize layerSize = VkDeviceSize(width) * height * 4;
    for (uint32_t i = 0; i < m_layerCount; ++i) {
        regions[i] = {};
        regions[i].bufferOffset = i * layerSize;
        regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        regions[i].imageSubresource.mipLevel = 0;
        regions[i].imageSubresource.baseArrayLayer = i;
        regions[i].imageSubresource.layerCount = 1;
        regions[i].imageExtent = {width, height, 1};
    }
    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(regions.size()), regions.data());

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<uint32_t>(regions.size()), regions.data());

    m_ctx->endSingleTimeCommands(commandBuffer);
}