#include "scene/Skybox.hpp"

#include "core/VulkanContext.hpp"
#include "core/Constants.hpp"
#include <stdexcept>
#include <fstream>
#include <string>
#include <cstring>

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("failed to open file: " + filename);
    size_t size = (size_t)file.tellg();
    std::vector<char> buffer(size);
    file.seekg(0);
    file.read(buffer.data(), size);
    return buffer;
}

static VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo info{.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                                  .codeSize = code.size(),
                                  .pCode = reinterpret_cast<const uint32_t*>(code.data())};
    VkShaderModule module;
    if (vkCreateShaderModule(device, &info, nullptr, &module) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module!");
    return module;
}

void Skybox::init(VulkanContext& ctx, VkRenderPass renderPass) {
    m_ctx = &ctx;
    createDescriptorSetLayout();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createPipeline(renderPass);
};

void Skybox::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding ubo{.binding = 0,
                                     .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     .descriptorCount = 1,
                                     .stageFlags = VK_SHADER_STAGE_VERTEX_BIT};

    VkDescriptorSetLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 1;
    info.pBindings = &ubo;
    if (vkCreateDescriptorSetLayout(m_ctx->device(), &info, nullptr, &m_setLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create sky descriptor set layout!");
}

void Skybox::createUniformBuffers() {
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

void Skybox::createDescriptorPool() {
    VkDevice device = m_ctx->device();
    VkDescriptorPoolSize poolSize{
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void Skybox::createDescriptorSets() {
    VkDevice device = m_ctx->device();
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_setLayout);

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

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = m_descriptorSets[i];
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount = 1;
        write.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    }
}

void Skybox::createPipeline(VkRenderPass renderPass) {
    VkDevice device = m_ctx->device();
    auto vertCode = readFile("shaders/sky_vert.spv"); // [4] shadery sky_*
    auto fragCode = readFile("shaders/sky_frag.spv");
    VkShaderModule vertModule = createShaderModule(device, vertCode);
    VkShaderModule fragModule = createShaderModule(device, fragCode);

    VkPipelineShaderStageCreateInfo vertStage{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                              .stage = VK_SHADER_STAGE_VERTEX_BIT,
                                              .module = vertModule,
                                              .pName = "main"};
    VkPipelineShaderStageCreateInfo fragStage{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                                              .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                                              .module = fragModule,
                                              .pName = "main"};
    VkPipelineShaderStageCreateInfo stages[] = {vertStage, fragStage};

    VkPipelineVertexInputStateCreateInfo vertexInput{.sType =
                                                         VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1};

    VkPipelineRasterizationStateCreateInfo rasterizer{.sType =
                                                          VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                                                      .depthClampEnable = VK_FALSE,
                                                      .rasterizerDiscardEnable = VK_FALSE,
                                                      .polygonMode = VK_POLYGON_MODE_FILL,
                                                      .cullMode = VK_CULL_MODE_NONE,
                                                      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                                                      .depthBiasEnable = VK_FALSE,
                                                      .lineWidth = 1.0f};

    VkPipelineMultisampleStateCreateInfo multisampling{.sType =
                                                           VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                                                       .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                                                       .sampleShadingEnable = VK_FALSE};

    VkPipelineDepthStencilStateCreateInfo depthStencil{.sType =
                                                           VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                                                       .depthTestEnable = VK_TRUE,
                                                       .depthWriteEnable = VK_FALSE,
                                                       .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
                                                       .depthBoundsTestEnable = VK_FALSE,
                                                       .stencilTestEnable = VK_FALSE};

    VkPipelineColorBlendAttachmentState colorBlendAttachment{.blendEnable = VK_FALSE,
                                                             .colorWriteMask =
                                                                 VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                                 VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
    VkPipelineColorBlendStateCreateInfo colorBlending{.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                                                      .logicOpEnable = VK_FALSE,
                                                      .attachmentCount = 1,
                                                      .pAttachments = &colorBlendAttachment};

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                                                  .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
                                                  .pDynamicStates = dynamicStates.data()};

    VkPipelineLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .setLayoutCount = 1, .pSetLayouts = &m_setLayout};
    if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create sky pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo{.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                                              .stageCount = 2,
                                              .pStages = stages,
                                              .pVertexInputState = &vertexInput,
                                              .pInputAssemblyState = &inputAssembly,
                                              .pViewportState = &viewportState,
                                              .pRasterizationState = &rasterizer,
                                              .pMultisampleState = &multisampling,
                                              .pDepthStencilState = &depthStencil,
                                              .pColorBlendState = &colorBlending,
                                              .pDynamicState = &dynamicState,
                                              .layout = m_pipelineLayout,
                                              .renderPass = renderPass,
                                              .subpass = 0,
                                              .basePipelineHandle = VK_NULL_HANDLE};
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS)
        throw std::runtime_error("failed to create sky pipeline!");

    vkDestroyShaderModule(device, fragModule, nullptr);
    vkDestroyShaderModule(device, vertModule, nullptr);
}

void Skybox::updateUniforms(uint32_t frame, const UniformBufferObject& ubo) {
    memcpy(m_uniformMapped[frame], &ubo, sizeof(ubo));
}

void Skybox::record(VkCommandBuffer cmd, uint32_t frame) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[frame], 0,
                            nullptr);
    vkCmdDraw(cmd, 3, 1, 0, 0); // 3 wierzchołki, zero bufora
}

void Skybox::cleanup() {
    VkDevice device = m_ctx->device();
    VmaAllocator allocator = m_ctx->allocator();
    vkDestroyPipeline(device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
    vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, m_setLayout, nullptr);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        vmaDestroyBuffer(allocator, m_uniformBuffers[i], m_uniformAllocations[i]);
}