#ifndef MAIN_H
#define MAIN_H

#include "../lib.h"

#ifdef VK_ZERO_CPU

#include <chrono>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <tuple>

#else

#endif
struct MainConstants {
    float4 color;
};

#ifdef VK_ZERO_CPU

std::optional<int> initialize() {
    if (auto result = volkInitialize(); result != VK_SUCCESS) {
        return -1;
    }
    if (auto result = glfwInit(); !result) {
        return -1;
    }
    if (!glfwVulkanSupported()) {
        return -1;
    }
    return {};
}

std::optional<int> create_window_instance_surface(const char *&name,
                                                  GLFWwindow *&window,
                                                  vkb::Instance &instance,
                                                  VkSurfaceKHR &surface) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(512, 512, name, nullptr, nullptr);
    if (!window) {
        return -1;
    }
    glfwSetWindowSizeLimits(window, 16, 16, GLFW_DONT_CARE, GLFW_DONT_CARE);
    uint32_t extensions_count = 0;
    const char **extensions =
        glfwGetRequiredInstanceExtensions(&extensions_count);
    vkb::InstanceBuilder instance_builder{};
    for (auto i = 0; i < extensions_count; ++i) {
        instance_builder.enable_extension(extensions[i]);
    }
#if !(NDEBUG)
    instance_builder.request_validation_layers();
    instance_builder.use_default_debug_messenger();
#endif
    if (auto result = instance_builder.set_app_name(name)
                          .require_api_version(1, 1)
                          .build();
        !result) {
        return -1;
    } else {
        instance = result.value();
    }
    volkLoadInstance(instance.instance);
    if (auto result = glfwCreateWindowSurface(instance.instance, window,
                                              nullptr, &surface);
        result != VK_SUCCESS) {
        std::cout << "...";
        return -1;
    }
    return {};
}

std::optional<int> create_device_allocator(const vkb::Instance &instance,
                                           const VkSurfaceKHR &surface,
                                           vkb::PhysicalDevice &physical_device,
                                           vkb::Device &device,
                                           VmaAllocator &allocator) {
    if (auto result = vkb::PhysicalDeviceSelector{instance}
                          .set_minimum_version(1, 1)
                          .set_required_features(
                              {.shaderStorageImageWriteWithoutFormat = VK_TRUE,
                               .shaderInt64 = VK_TRUE})
                          .set_required_features_11(
                              {.variablePointersStorageBuffer = VK_TRUE,
                               .variablePointers = VK_TRUE})
                          .add_desired_extension("VK_KHR_portability_subset")
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
    VmaAllocatorCreateInfo create_info = {
        .physicalDevice = physical_device.physical_device,
        .device = device.device,
        .instance = instance.instance,
        .vulkanApiVersion = VK_API_VERSION_1_1};
    if (vmaCreateAllocator(&create_info, &allocator) != VK_SUCCESS) {
        return -1;
    }
    return {};
}

std::optional<int> get_graphics_compute_queue(const vkb::Device &device,
                                              VkQueue &graphics_queue,
                                              uint32_t &graphics_queue_index,
                                              VkQueue &compute_queue,
                                              uint32_t &compute_queue_index) {
    if (auto result = device.get_queue_index(vkb::QueueType::present);
        !result.has_value()) {
        return -1;
    } else {
        graphics_queue_index = result.value();
    }
    vkGetDeviceQueue(device.device, graphics_queue_index, 0, &graphics_queue);
    if (auto result = device.get_queue_index(vkb::QueueType::present);
        !result.has_value()) {
        return -1;
    } else {
        compute_queue_index = result.value();
    }
    vkGetDeviceQueue(device.device, compute_queue_index, 0, &compute_queue);
    return {};
}

std::optional<int> create_command_pool(const vkb::Device &device,
                                       const uint32_t &queue_index,
                                       VkCommandPool &command_pool) {
    VkCommandPoolCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queue_index};
    if (vkCreateCommandPool(device.device, &create_info, nullptr,
                            &command_pool) != VK_SUCCESS) {
        return -1;
    }
    return {};
}

std::optional<int> create_descriptor_pool(const vkb::Device &device,
                                          VkDescriptorPool &descriptor_pool) {
    std::vector<VkDescriptorPoolSize> pool_sizes{
        {VK_DESCRIPTOR_TYPE_SAMPLER, 1024},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1024},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1024},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1024},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1024}};
    VkDescriptorPoolCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1024,
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data()};
    if (vkCreateDescriptorPool(device.device, &create_info, nullptr,
                               &descriptor_pool) != VK_SUCCESS) {
        return -1;
    }
    return {};
}

std::optional<int> create_buffer_uniform(const VmaAllocator &allocator,
                                         const VkDeviceSize &size,
                                         VkBuffer &buffer,
                                         VmaAllocation &allocation,
                                         VmaAllocationInfo &allocation_info) {
    VkBufferCreateInfo buffer_create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_create_info.size = size;
    buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
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
         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
         .descriptorCount = 1,
         .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
         .pImmutableSamplers = nullptr},
        {.binding = 1,
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
    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t *>(code.data())};
    if (vkCreateShaderModule(device.device, &create_info, nullptr,
                             &shader_module) != VK_SUCCESS) {
        return -1;
    }
    return {};
}

std::optional<int> create_pipeline(const vkb::Device &device,
                                   const VkPipelineLayout &pipeline_layout,
                                   const VkShaderModule &shader_module,
                                   const uint3 &local_size, const char *&name,
                                   VkPipeline &pipeline) {
    VkSpecializationMapEntry map_entries[3]{{.constantID = 0,
                                             .offset = sizeof(uint32_t) * 0,
                                             .size = sizeof(uint32_t)},
                                            {.constantID = 1,
                                             .offset = sizeof(uint32_t) * 1,
                                             .size = sizeof(uint32_t)},
                                            {.constantID = 2,
                                             .offset = sizeof(uint32_t) * 2,
                                             .size = sizeof(uint32_t)}};
    VkSpecializationInfo specialization_info{.mapEntryCount = 3,
                                             .pMapEntries = map_entries,
                                             .dataSize = sizeof(local_size),
                                             .pData = &local_size};
    VkComputePipelineCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                  .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                  .module = shader_module,
                  .pName = name,
                  .pSpecializationInfo = &specialization_info},
        .layout = pipeline_layout,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = 0};
    if (vkCreateComputePipelines(device.device, nullptr, 1, &create_info,
                                 nullptr, &pipeline) != VK_SUCCESS) {
        return -1;
    }
    return {};
}

std::optional<int> create_swapchain_semaphores_fences_render_pass_framebuffers(
    const vkb::Device &device, vkb::Swapchain &swapchain,
    std::vector<VkImage> &images, std::vector<VkImageView> &image_views,
    std::vector<VkFence> &signal_fences,
    std::vector<VkSemaphore> &wait_semaphores,
    std::vector<VkSemaphore> &signal_semaphores, VkRenderPass &render_pass,
    std::vector<VkFramebuffer> &framebuffers, bool destroy = false) {
    auto builder =
        vkb::SwapchainBuilder{device}
            .add_fallback_format(
                {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
            .add_fallback_format(
                {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
            .add_format_feature_flags(VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
            .add_format_feature_flags(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
            .add_image_usage_flags(VK_IMAGE_USAGE_STORAGE_BIT)
            .add_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    if (destroy) {
        builder.set_old_swapchain(swapchain);
    }
    if (auto result = builder.build(); !result) {
        return -1;
    } else {
        if (destroy) {
            for (auto &framebuffer : framebuffers) {
                vkDestroyFramebuffer(device.device, framebuffer, nullptr);
            }
            vkDestroyRenderPass(device.device, render_pass, nullptr);
        }
        swapchain.destroy_image_views(image_views);
        vkb::destroy_swapchain(swapchain);
        swapchain = result.value();
    }
    images = swapchain.get_images().value();
    image_views = swapchain.get_image_views().value();
    if (destroy) {
        for (auto &fence : signal_fences) {
            vkDestroyFence(device.device, fence, nullptr);
        }
        for (auto &semaphore : wait_semaphores) {
            vkDestroySemaphore(device.device, semaphore, nullptr);
        }
        for (auto &semaphore : signal_semaphores) {
            vkDestroySemaphore(device.device, semaphore, nullptr);
        }
    }
    signal_fences = std::vector<VkFence>{swapchain.image_count * 2};
    wait_semaphores = std::vector<VkSemaphore>{swapchain.image_count};
    signal_semaphores = std::vector<VkSemaphore>{swapchain.image_count * 2};
    VkFenceCreateInfo fence_info = {.sType =
                                        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                    .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    VkSemaphoreCreateInfo semaphore_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (auto i = 0; i < swapchain.image_count; i++) {
        if (vkCreateSemaphore(device.device, &semaphore_info, nullptr,
                              &wait_semaphores[i]) != VK_SUCCESS) {
            return -1;
        }
    }
    for (auto i = 0; i < swapchain.image_count * 2; i++) {
        if (vkCreateFence(device.device, &fence_info, nullptr,
                          &signal_fences[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device.device, &semaphore_info, nullptr,
                              &signal_semaphores[i]) != VK_SUCCESS) {
            return -1;
        }
    }
    VkAttachmentDescription attachment = {
        .format = swapchain.image_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_GENERAL};
    VkAttachmentReference color_attachment = {
        .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass = {.pipelineBindPoint =
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    .colorAttachmentCount = 1,
                                    .pColorAttachments = &color_attachment};
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                         VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};
    VkRenderPassCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency};
    if (vkCreateRenderPass(device.device, &create_info, nullptr,
                           &render_pass) != VK_SUCCESS) {
        return -1;
    }
    framebuffers = std::vector<VkFramebuffer>{swapchain.image_count};
    for (auto i = 0; i < swapchain.image_count; i++) {
        VkFramebufferCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = render_pass,
            .attachmentCount = 1,
            .pAttachments = &image_views[i],
            .width = swapchain.extent.width,
            .height = swapchain.extent.height,
            .layers = 1};
        if (vkCreateFramebuffer(device.device, &create_info, nullptr,
                                &framebuffers[i]) != VK_SUCCESS) {
            return -1;
        }
    }
    return {};
}

std::optional<int>
allocate_descriptor_sets(const vkb::Device &device,
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
    std::vector<VkDescriptorImageInfo> image_info{swapchain.image_count};
    std::vector<VkDescriptorBufferInfo> buffer_info{swapchain.image_count};
    std::vector<VkWriteDescriptorSet> descriptor_writes{swapchain.image_count *
                                                        2};
    for (auto i = 0; i < swapchain.image_count; ++i) {
        image_info[i] = {.imageView = image_views[i],
                         .imageLayout = VK_IMAGE_LAYOUT_GENERAL};
        buffer_info[i] = {
            .buffer = buffer_uniform, .offset = 0, .range = VK_WHOLE_SIZE};
        descriptor_writes[i * 2 + 0] = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = descriptor_sets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .pImageInfo = &image_info[i],
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr};
        descriptor_writes[i * 2 + 1] = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = descriptor_sets[i],
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &buffer_info[i],
            .pTexelBufferView = nullptr};
    }
    vkUpdateDescriptorSets(device.device, descriptor_writes.size(),
                           descriptor_writes.data(), 0, nullptr);
    return {};
}

std::optional<int> allocate_command_buffers(
    GLFWwindow *&window, const vkb::Device &device, const uint32_t &queue_index,
    const VkCommandPool &command_pool, const vkb::Swapchain &swapchain,
    std::vector<VkCommandBuffer> &command_buffers) {
    command_buffers = std::vector<VkCommandBuffer>{swapchain.image_count};
    VkCommandBufferAllocateInfo allocate_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = swapchain.image_count};
    if (vkAllocateCommandBuffers(device.device, &allocate_info,
                                 command_buffers.data()) != VK_SUCCESS) {
        return -1;
    }
    return {};
}

std::optional<int> imgui_initialize(
    GLFWwindow *&window, vkb::Instance &instance,
    const vkb::PhysicalDevice &physical_device, const vkb::Device &device,
    const VkQueue &queue, const uint32_t &queue_index,
    const VkDescriptorPool &descriptor_pool, const vkb::Swapchain &swapchain,
    const VkRenderPass &render_pass, const VkCommandBuffer &command_buffer,
    ImGuiIO &imgui_io) {
    ImGui::CreateContext();
    imgui_io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(window, false);
    ImGui_ImplVulkan_LoadFunctions(
        [](const char *function_name, void *instance) {
            return vkGetInstanceProcAddr(*static_cast<VkInstance *>(instance),
                                         function_name);
        },
        static_cast<void *>(&instance.instance));
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = instance.instance;
    init_info.PhysicalDevice = physical_device.physical_device;
    init_info.Device = device.device;
    init_info.QueueFamily = queue_index;
    init_info.Queue = queue;
    init_info.DescriptorPool = descriptor_pool;
    init_info.MinImageCount = swapchain.image_count;
    init_info.ImageCount = swapchain.image_count;
    if (!ImGui_ImplVulkan_Init(&init_info, render_pass)) {
        return -1;
    }
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &begin_info);
    ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
    VkSubmitInfo end_info = {};
    end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = &command_buffer;
    vkEndCommandBuffer(command_buffer);
    vkQueueSubmit(queue, 1, &end_info, nullptr);
    vkDeviceWaitIdle(device.device);
    vkResetCommandBuffer(command_buffer,
                         VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
    vkDeviceWaitIdle(device.device);
    return {};
}

std::optional<int> frame_submit(
    const vkb::Device &device, const VkQueue &graphics_queue,
    const VkQueue &compute_queue, const vkb::Swapchain &swapchain,
    const std::vector<VkFence> &signal_fences,
    const std::vector<VkSemaphore> &wait_semaphores,
    const std::vector<VkSemaphore> &signal_semaphores,
    const std::vector<VkCommandBuffer> &graphics_command_buffers,
    const std::vector<VkCommandBuffer> &compute_command_buffers,
    uint32_t &index,
    std::function<std::optional<int>(const uint32_t &, const VkCommandBuffer &)>
        graphics_commands,
    std::function<std::optional<int>(const uint32_t &, const VkCommandBuffer &)>
        compute_commands) {
    uint32_t image_index;
    VkResult result =
        vkAcquireNextImageKHR(device.device, swapchain.swapchain, UINT64_MAX,
                              wait_semaphores[index], nullptr, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return 0;
    } else if (result == VK_NOT_READY || result == VK_TIMEOUT) {
        return 1;
    } else if (result != VK_SUCCESS) {
        return -1;
    }
    if (vkWaitForFences(device.device, 2, &signal_fences[image_index * 2],
                        VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
        return -1;
    }
    if (vkResetFences(device.device, 2, &signal_fences[image_index * 2]) !=
        VK_SUCCESS) {
        return -1;
    }
    if (vkResetCommandBuffer(graphics_command_buffers[image_index],
                             VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) !=
        VK_SUCCESS) {
        return -1;
    }
    VkCommandBufferBeginInfo graphics_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
    if (vkBeginCommandBuffer(graphics_command_buffers[image_index],
                             &graphics_begin_info) != VK_SUCCESS) {
        return -1;
    }
    if (auto error = graphics_commands(image_index,
                                       graphics_command_buffers[image_index])) {
        return -1;
    }
    if (vkEndCommandBuffer(graphics_command_buffers[image_index]) !=
        VK_SUCCESS) {
        return -1;
    }
    VkPipelineStageFlags graphics_wait_stages[] = {
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
    VkSubmitInfo graphics_submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &wait_semaphores[index],
        .pWaitDstStageMask = graphics_wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &graphics_command_buffers[image_index],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &signal_semaphores[image_index * 2 + 0]};
    if (vkQueueSubmit(graphics_queue, 1, &graphics_submit_info,
                      signal_fences[image_index * 2 + 0]) != VK_SUCCESS) {
        return -1;
    }
    if (vkResetCommandBuffer(compute_command_buffers[image_index],
                             VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) !=
        VK_SUCCESS) {
        return -1;
    }
    VkCommandBufferBeginInfo compute_begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
    if (vkBeginCommandBuffer(compute_command_buffers[image_index],
                             &compute_begin_info) != VK_SUCCESS) {
        return -1;
    }
    if (auto error = compute_commands(image_index,
                                      compute_command_buffers[image_index])) {
        return -1;
    }
    if (vkEndCommandBuffer(compute_command_buffers[image_index]) !=
        VK_SUCCESS) {
        return -1;
    }
    VkPipelineStageFlags compute_wait_stages[] = {
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
    VkSubmitInfo compute_submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &signal_semaphores[image_index * 2 + 0],
        .pWaitDstStageMask = compute_wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &compute_command_buffers[image_index],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &signal_semaphores[image_index * 2 + 1]};
    if (vkQueueSubmit(compute_queue, 1, &compute_submit_info,
                      signal_fences[image_index * 2 + 1]) != VK_SUCCESS) {
        return -1;
    }
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &signal_semaphores[image_index * 2 + 1],
        .swapchainCount = 1,
        .pSwapchains = &swapchain.swapchain,
        .pImageIndices = &image_index};
    result = vkQueuePresentKHR(graphics_queue, &present_info);
    index = image_index;
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return 0;
    } else if (result != VK_SUCCESS) {
        return -1;
    }
    return {};
}

#endif

#endif
