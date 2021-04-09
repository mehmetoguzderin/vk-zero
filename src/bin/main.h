#include "lib.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

const uint32_t GROUP_SIZE = 16;

std::optional<int> initialize() {
    if (auto result = volkInitialize(); result != VK_SUCCESS) {
        return -1;
    }
    if (auto result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS |
                               SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER);
        result < 0) {
        return -1;
    }
    return {};
}

std::optional<int> create_window_instance_surface(const char *&name,
                                                  SDL_Window *&window,
                                                  vkb::Instance &instance,
                                                  VkSurfaceKHR &surface) {
    window = SDL_CreateWindow(
        name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) {
        return -1;
    }
    SDL_SetWindowMinimumSize(window, 16, 16);
    uint32_t extensions_count = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, NULL);
    std::vector<const char *> extensions{extensions_count};
    SDL_Vulkan_GetInstanceExtensions(window, &extensions_count,
                                     extensions.data());
    vkb::InstanceBuilder instance_builder{};
    for (auto &extension : extensions) {
        instance_builder.enable_extension(extension);
    }
    if (auto result = instance_builder.set_app_name(name)
                          .require_api_version(1, 1)
                          .use_default_debug_messenger()
                          .request_validation_layers()
                          .build();
        !result) {
        return -1;
    } else {
        instance = result.value();
    }
    volkLoadInstance(instance.instance);
    if (auto result =
            SDL_Vulkan_CreateSurface(window, instance.instance, &surface);
        !result) {
        return -1;
    }
    return {};
}

std::optional<int> create_device(const vkb::Instance &instance,
                                 const VkSurfaceKHR &surface,
                                 vkb::PhysicalDevice physical_device,
                                 vkb::Device &device) {
    if (auto result =
            vkb::PhysicalDeviceSelector{instance}
                .set_minimum_version(1, 1)
                .add_required_extension(
                    VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME)
                .add_required_extension(VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME)
                .set_surface(surface)
                .select();
        !result) {
        return -1;
    } else {
        physical_device = result.value();
    }
    if (auto result = vkb::DeviceBuilder{physical_device}.build(); !result) {
        return -1;
    } else {
        device = result.value();
    }
    volkLoadDevice(device.device);
    return {};
}

std::optional<int> get_queue(const vkb::Device &device, VkQueue &queue,
                             uint32_t &queue_index) {
    if (auto result = device.get_queue(vkb::QueueType::present);
        !result.has_value()) {
        return -1;
    } else {
        queue = result.value();
    }
    queue_index = device.get_queue_index(vkb::QueueType::present).value();
    return {};
}

std::optional<int> create_command_pool(const vkb::Device &device,
                                       const uint32_t &queue_index,
                                       VkCommandPool &command_pool) {
    VkCommandPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = queue_index;
    if (vkCreateCommandPool(device.device, &create_info, VK_NULL_HANDLE,
                            &command_pool) != VK_SUCCESS) {
        return -1;
    }
    return {};
}

std::optional<int>
create_set_pipeline_layout(const vkb::Device &device,
                           VkDescriptorSetLayout &set_layout,
                           VkPipelineLayout &pipeline_layout) {
    VkDescriptorSetLayoutBinding binding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .pImmutableSamplers = VK_NULL_HANDLE};
    VkDescriptorSetLayoutCreateInfo set_create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = VK_NULL_HANDLE,
        .bindingCount = 1,
        .pBindings = &binding};
    if (vkCreateDescriptorSetLayout(device.device, &set_create_info,
                                    VK_NULL_HANDLE,
                                    &set_layout) != VK_SUCCESS) {
        return -1;
    }
    VkPipelineLayoutCreateInfo pipeline_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = VK_NULL_HANDLE,
        .setLayoutCount = 1,
        .pSetLayouts = &set_layout,
        .pushConstantRangeCount = VK_NULL_HANDLE,
        .pPushConstantRanges = VK_NULL_HANDLE};
    if (vkCreatePipelineLayout(device.device, &pipeline_create_info,
                               VK_NULL_HANDLE,
                               &pipeline_layout) != VK_SUCCESS) {
        return -1;
    }
    return {};
}

std::optional<int> create_shader_module(const vkb::Device &device,
                                        const char *&name,
                                        VkShaderModule &shader_module) {
    std::ifstream file(name, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        return -1;
    }
    size_t file_size = (size_t)file.tellg();
    std::vector<char> code(file_size);
    file.seekg(0);
    file.read(code.data(), static_cast<std::streamsize>(file_size));
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());
    if (vkCreateShaderModule(device.device, &create_info, VK_NULL_HANDLE,
                             &shader_module) != VK_SUCCESS) {
        return -1;
    }
    return {};
}

std::optional<int> create_pipeline(const vkb::Device &device,
                                   const VkPipelineLayout &pipeline_layout,
                                   const VkShaderModule &shader_module,
                                   const char *&name, VkPipeline &pipeline) {
    VkSpecializationMapEntry map_entries[3]{{.constantID = 0,
                                             .offset = sizeof(uint32_t) * 0,
                                             .size = sizeof(uint32_t)},
                                            {.constantID = 1,
                                             .offset = sizeof(uint32_t) * 1,
                                             .size = sizeof(uint32_t)},
                                            {.constantID = 2,
                                             .offset = sizeof(uint32_t) * 2,
                                             .size = sizeof(uint32_t)}};
    uint32_t data[3] = {GROUP_SIZE, GROUP_SIZE, 1};
    VkSpecializationInfo specialization_info{.mapEntryCount = 3,
                                             .pMapEntries = map_entries,
                                             .dataSize = sizeof(uint32_t) * 3,
                                             .pData = data};
    VkComputePipelineCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .stage = {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                  .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                  .module = shader_module,
                  .pName = name,
                  .pSpecializationInfo = &specialization_info},
        .layout = pipeline_layout,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = VK_NULL_HANDLE,
    };
    if (vkCreateComputePipelines(device.device, VK_NULL_HANDLE, 1, &create_info,
                                 VK_NULL_HANDLE, &pipeline) != VK_SUCCESS) {
        return -1;
    }
    return {};
}

std::optional<int> create_swapchain_semaphores_fences(
    const vkb::Device &device, vkb::Swapchain &swapchain,
    std::vector<VkImage> &images, std::vector<VkImageView> &image_views,
    std::vector<VkFence> &signal_fences,
    std::vector<VkSemaphore> &wait_semaphores,
    std::vector<VkSemaphore> &signal_semaphores,
    std::vector<VkFence> &wait_fences, bool destroy = false) {
    auto builder =
        vkb::SwapchainBuilder{device}
            .use_default_format_selection()
            .add_fallback_format(
                {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
            .add_fallback_format(
                {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
            .add_format_feature_flags(VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
            .add_image_usage_flags(VK_IMAGE_USAGE_STORAGE_BIT);
    if (destroy) {
        builder.set_old_swapchain(swapchain);
    }
    if (auto result = builder.build(); !result) {
        return -1;
    } else {
        vkb::destroy_swapchain(swapchain);
        swapchain = result.value();
    }
    images = swapchain.get_images().value();
    image_views = swapchain.get_image_views().value();
    if (destroy) {
        for (auto &fence : signal_fences) {
            vkDestroyFence(device.device, fence, VK_NULL_HANDLE);
        }
        for (auto &semaphore : wait_semaphores) {
            vkDestroySemaphore(device.device, semaphore, VK_NULL_HANDLE);
        }
        for (auto &semaphore : signal_semaphores) {
            vkDestroySemaphore(device.device, semaphore, VK_NULL_HANDLE);
        }
    }
    wait_fences = std::vector<VkFence>{swapchain.image_count, VK_NULL_HANDLE};
    signal_fences = std::vector<VkFence>{swapchain.image_count};
    wait_semaphores = std::vector<VkSemaphore>{swapchain.image_count};
    signal_semaphores = std::vector<VkSemaphore>{swapchain.image_count};
    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (auto i = 0; i < swapchain.image_count; i++) {
        if (vkCreateFence(device.device, &fence_info, VK_NULL_HANDLE,
                          &signal_fences[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device.device, &semaphore_info, VK_NULL_HANDLE,
                              &wait_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device.device, &semaphore_info, VK_NULL_HANDLE,
                              &signal_semaphores[i]) != VK_SUCCESS) {
            return -1;
        }
    }
    return {};
}

std::optional<int> create_descriptor_pool(const vkb::Device &device,
                                          const vkb::Swapchain &swapchain,
                                          VkDescriptorPool &descriptor_pool) {
    VkDescriptorPoolSize pool_size{.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                   .descriptorCount = swapchain.image_count};
    VkDescriptorPoolCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = VK_NULL_HANDLE,
        .maxSets = swapchain.image_count,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size};
    if (vkCreateDescriptorPool(device.device, &create_info, VK_NULL_HANDLE,
                               &descriptor_pool) != VK_SUCCESS) {
        return -1;
    }
    return {};
}

std::optional<int>
allocate_descriptor_sets(const vkb::Device &device,
                         const vkb::Swapchain &swapchain,
                         const std::vector<VkImageView> &image_views,
                         const VkDescriptorSetLayout &set_layout,
                         const VkDescriptorPool &descriptor_pool,
                         std::vector<VkDescriptorSet> &descriptor_sets) {
    std::vector<VkDescriptorSetLayout> set_layouts{swapchain.image_count,
                                                   set_layout};
    VkDescriptorSetAllocateInfo allocate_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .descriptorPool = descriptor_pool,
        .descriptorSetCount = swapchain.image_count,
        .pSetLayouts = set_layouts.data()};
    descriptor_sets = std::vector<VkDescriptorSet>{swapchain.image_count};
    if (vkAllocateDescriptorSets(device.device, &allocate_info,
                                 descriptor_sets.data()) != VK_SUCCESS) {
        return -1;
    }
    std::vector<VkDescriptorImageInfo> image_info{swapchain.image_count};
    std::vector<VkWriteDescriptorSet> descriptor_writes{swapchain.image_count};
    for (auto i = 0; i < swapchain.image_count; ++i) {
        image_info[i] = {.imageView = image_views[i],
                         .imageLayout = VK_IMAGE_LAYOUT_GENERAL};
        descriptor_writes[i] = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                                .pNext = VK_NULL_HANDLE,
                                .dstSet = descriptor_sets[i],
                                .dstBinding = 0,
                                .dstArrayElement = 0,
                                .descriptorCount = 1,
                                .descriptorType =
                                    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                                .pImageInfo = &image_info[i],
                                .pBufferInfo = VK_NULL_HANDLE,
                                .pTexelBufferView = VK_NULL_HANDLE};
    }
    vkUpdateDescriptorSets(device.device, descriptor_writes.size(),
                           descriptor_writes.data(), 0, VK_NULL_HANDLE);
    return {};
}

std::optional<int> allocate_command_buffers(
    SDL_Window *&window, const vkb::Device &device, const uint32_t &queue_index,
    const VkCommandPool &command_pool, const VkPipelineLayout &pipeline_layout,
    const VkPipeline &pipeline, const vkb::Swapchain &swapchain,
    const std::vector<VkImage> &images,
    const std::vector<VkImageView> &image_views,
    const std::vector<VkDescriptorSet> &descriptor_sets,
    std::vector<VkCommandBuffer> &command_buffers) {
    command_buffers = std::vector<VkCommandBuffer>{swapchain.image_count};
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
    command_buffer_allocate_info.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = swapchain.image_count;
    if (vkAllocateCommandBuffers(device.device, &command_buffer_allocate_info,
                                 command_buffers.data()) != VK_SUCCESS) {
        return -1;
    }
    int width, height;
    SDL_Vulkan_GetDrawableSize(window, &width, &height);
    for (auto i = 0; i < swapchain.image_count; i++) {
        VkCommandBufferBeginInfo command_buffer_begin_info = {};
        command_buffer_begin_info.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(command_buffers[i],
                                 &command_buffer_begin_info) != VK_SUCCESS) {
            return -1;
        }
        vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_COMPUTE,
                          pipeline);
        vkCmdBindDescriptorSets(
            command_buffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout,
            0, 1, &descriptor_sets[i], VK_NULL_HANDLE, VK_NULL_HANDLE);
        VkImageMemoryBarrier begin_image_memory_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = VK_NULL_HANDLE,
            .srcAccessMask = VK_NULL_HANDLE,
            .dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .srcQueueFamilyIndex = queue_index,
            .dstQueueFamilyIndex = queue_index,
            .image = images[i],
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                 .baseMipLevel = VK_NULL_HANDLE,
                                 .levelCount = 1,
                                 .baseArrayLayer = VK_NULL_HANDLE,
                                 .layerCount = 1}};
        vkCmdPipelineBarrier(
            command_buffers[i], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_NULL_HANDLE,
            VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
            &begin_image_memory_barrier);
        vkCmdDispatch(command_buffers[i], width / GROUP_SIZE + 1,
                      height / GROUP_SIZE + 1, 1);
        VkImageMemoryBarrier end_image_memory_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = VK_NULL_HANDLE,
            .srcAccessMask = VK_NULL_HANDLE,
            .dstAccessMask = VK_NULL_HANDLE,
            .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = queue_index,
            .dstQueueFamilyIndex = queue_index,
            .image = images[i],
            .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                 .baseMipLevel = VK_NULL_HANDLE,
                                 .levelCount = 1,
                                 .baseArrayLayer = VK_NULL_HANDLE,
                                 .layerCount = 1}};
        vkCmdPipelineBarrier(
            command_buffers[i], VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_NULL_HANDLE,
            VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, 1,
            &end_image_memory_barrier);
        if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS) {
            return -1;
        }
    }
    return {};
}

std::optional<int>
queue_submit(const vkb::Device &device, const VkQueue &queue,
             const vkb::Swapchain &swapchain,
             const std::vector<VkFence> &signal_fences,
             const std::vector<VkSemaphore> &wait_semaphores,
             const std::vector<VkSemaphore> &signal_semaphores,
             const std::vector<VkCommandBuffer> &command_buffers,
             const uint32_t &index, std::vector<VkFence> &wait_fences) {
    if (vkWaitForFences(device.device, 1, &signal_fences[index], VK_TRUE,
                        UINT64_MAX) != VK_SUCCESS) {
        return -1;
    }
    uint32_t image_index = 0;
    VkResult result = vkAcquireNextImageKHR(device.device, swapchain.swapchain,
                                            UINT64_MAX, wait_semaphores[index],
                                            VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return 0;
    } else if (result != VK_SUCCESS) {
        return -1;
    }
    if (wait_fences[image_index] != VK_NULL_HANDLE) {
        if (vkWaitForFences(device.device, 1, &wait_fences[image_index],
                            VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
            return -1;
        }
    }
    wait_fences[image_index] = signal_fences[index];
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &wait_semaphores[index];
    submitInfo.pWaitDstStageMask = wait_stages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &command_buffers[image_index];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signal_semaphores[index];
    if (vkResetFences(device.device, 1, &signal_fences[index]) != VK_SUCCESS) {
        return -1;
    }
    if (vkQueueSubmit(queue, 1, &submitInfo, signal_fences[index]) !=
        VK_SUCCESS) {
        return -1;
    }
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &signal_semaphores[index];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain.swapchain;
    present_info.pImageIndices = &image_index;
    result = vkQueuePresentKHR(queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return 0;
    } else if (result != VK_SUCCESS) {
        return -1;
    }
    return {};
}