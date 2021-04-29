#ifndef COMPUTE_WEIGHTED_ADD_H
#define COMPUTE_WEIGHTED_ADD_H

#include "main.h"

#ifdef VK_ZERO_CPU

#else

#endif

#define ELEMENT_WIDTH 2048

struct ComputeWeightedAddElement {
    float4 element[ELEMENT_WIDTH];
};
constexpr uint64_t ELEMENT_SIZE = sizeof(float4);

struct ComputeWeightedAddConstants {
    float4 weights;
    uint2 length;
};

#ifdef VK_ZERO_CPU

namespace compute_weighted_add {
std::optional<int> create_buffer_storage(const VmaAllocator &allocator,
                                         const VkDeviceSize &size,
                                         VkBuffer &buffer,
                                         VmaAllocation &allocation,
                                         VmaAllocationInfo &allocation_info) {
    VkBufferCreateInfo buffer_create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_create_info.size = size;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VmaAllocationCreateInfo allocation_create_info = {};
    allocation_create_info.usage = VMA_MEMORY_USAGE_UNKNOWN;
    allocation_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocation_create_info.requiredFlags =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (vmaCreateBuffer(allocator, &buffer_create_info, &allocation_create_info,
                        &buffer, &allocation, &allocation_info) != VK_SUCCESS) {
        return -1;
    }
    return {};
}

std::optional<int>
create_set_pipeline_layout(const vkb::Device &device,
                           VkDescriptorSetLayout &set_layout,
                           VkPipelineLayout &pipeline_layout) {
    std::vector<VkDescriptorSetLayoutBinding> bindings{
        {.binding = 0,
         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
         .pImmutableSamplers = nullptr},
        {.binding = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
         .pImmutableSamplers = nullptr},
        {.binding = 2,
         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
         .pImmutableSamplers = nullptr},
        {.binding = 3,
         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
         .pImmutableSamplers = nullptr},
        {.binding = 4,
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
         .pImmutableSamplers = nullptr}};
    VkDescriptorSetLayoutCreateInfo set_create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = (uint32_t)bindings.size(),
        .pBindings = bindings.data()};
    if (vkCreateDescriptorSetLayout(device.device, &set_create_info, nullptr,
                                    &set_layout) != VK_SUCCESS) {
        return -1;
    }
    VkPipelineLayoutCreateInfo pipeline_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr};
    if (vkCreatePipelineLayout(device.device, &pipeline_create_info, nullptr,
                               &pipeline_layout) != VK_SUCCESS) {
        return -1;
    }
    return {};
}

std::optional<int>
allocate_descriptor_sets(const vkb::Device &device,
                         const VkBuffer &buffer_storage_a,
                         const VmaAllocationInfo &allocation_info_storage_a,
                         const VkBuffer &buffer_storage_b,
                         const VmaAllocationInfo &allocation_info_storage_b,
                         const VkBuffer &buffer_storage_c,
                         const VmaAllocationInfo &allocation_info_storage_c,
                         const VkBuffer &buffer_storage_d,
                         const VmaAllocationInfo &allocation_info_storage_d,
                         const VkBuffer &buffer_uniform,
                         const VmaAllocationInfo &allocation_info_uniform,
                         const vkb::Swapchain &swapchain,
                         const std::vector<VkImageView> &image_views,
                         const VkDescriptorSetLayout &set_layout,
                         const VkDescriptorPool &descriptor_pool,
                         std::vector<VkDescriptorSet> &descriptor_sets) {
    std::vector<VkDescriptorSetLayout> set_layouts{swapchain.image_count,
                                                   set_layout};
    VkDescriptorSetAllocateInfo allocate_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = descriptor_pool,
        .descriptorSetCount = swapchain.image_count,
        .pSetLayouts = set_layouts.data()};
    descriptor_sets = std::vector<VkDescriptorSet>{swapchain.image_count};
    if (vkAllocateDescriptorSets(device.device, &allocate_info,
                                 descriptor_sets.data()) != VK_SUCCESS) {
        return -1;
    }
    std::vector<VkDescriptorBufferInfo> buffer_info{swapchain.image_count * 5};
    std::vector<VkWriteDescriptorSet> descriptor_writes{swapchain.image_count *
                                                        5};
    for (auto i = 0; i < swapchain.image_count; ++i) {
        buffer_info[i * 5 + 0] = {
            .buffer = buffer_storage_a, .offset = 0, .range = VK_WHOLE_SIZE};
        descriptor_writes[i * 5 + 0] = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = descriptor_sets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &buffer_info[i * 5 + 0],
            .pTexelBufferView = nullptr};
        buffer_info[i * 5 + 1] = {
            .buffer = buffer_storage_b, .offset = 0, .range = VK_WHOLE_SIZE};
        descriptor_writes[i * 5 + 1] = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = descriptor_sets[i],
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &buffer_info[i * 5 + 1],
            .pTexelBufferView = nullptr};
        buffer_info[i * 5 + 2] = {
            .buffer = buffer_storage_c, .offset = 0, .range = VK_WHOLE_SIZE};
        descriptor_writes[i * 5 + 2] = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = descriptor_sets[i],
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &buffer_info[i * 5 + 2],
            .pTexelBufferView = nullptr};
        buffer_info[i * 5 + 3] = {
            .buffer = buffer_storage_d, .offset = 0, .range = VK_WHOLE_SIZE};
        descriptor_writes[i * 5 + 3] = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = descriptor_sets[i],
            .dstBinding = 3,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &buffer_info[i * 5 + 3],
            .pTexelBufferView = nullptr};
        buffer_info[i * 5 + 4] = {
            .buffer = buffer_uniform, .offset = 0, .range = VK_WHOLE_SIZE};
        descriptor_writes[i * 5 + 4] = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = descriptor_sets[i],
            .dstBinding = 4,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &buffer_info[i * 5 + 4],
            .pTexelBufferView = nullptr};
    }
    vkUpdateDescriptorSets(device.device, descriptor_writes.size(),
                           descriptor_writes.data(), 0, nullptr);
    return {};
}
} // namespace compute_weighted_add

#else

#endif

#endif