#include "main.h"

int main(int argc, char *argv[]) {
    if (auto error = initialize()) {
        return -1;
    }
    auto name = "main";
    SDL_Window *window;
    vkb::Instance instance;
    VkSurfaceKHR surface;
    vkb::Device device;
    if (auto error = create_window_instance_surface_device(
            name, window, instance, surface, device)) {
        return -1;
    }
    vkb::Swapchain swapchain;
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
    if (auto error = create_swapchain(device, swapchain, images, image_views)) {
        return -1;
    }
    VkDescriptorPoolSize descriptor_pool_size{
        .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = (uint32_t)image_views.size()};
    VkDescriptorPoolCreateInfo descriptor_pool_create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .maxSets = 3,
        .poolSizeCount = 1,
        .pPoolSizes = &descriptor_pool_size};
    VkDescriptorPool descriptor_pool;
    if (vkCreateDescriptorPool(device.device, &descriptor_pool_create_info,
                               VK_NULL_HANDLE,
                               &descriptor_pool) != VK_SUCCESS) {
        std::cerr << "failed to create descriptor pool"
                  << "\n";
        return 1;
    }
    VkDescriptorSetLayoutBinding descriptor_set_layout_binding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .pImmutableSamplers = VK_NULL_HANDLE};
    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .bindingCount = 1,
        .pBindings = &descriptor_set_layout_binding};
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts{
        image_views.size()};
    if (vkCreateDescriptorSetLayout(
            device.device, &descriptor_set_layout_create_info, VK_NULL_HANDLE,
            descriptor_set_layouts.data()) != VK_SUCCESS) {
        std::cerr << "failed to create descriptor set layout"
                  << "\n";
        return 1;
    }
    for (auto i = 1; i < descriptor_set_layouts.size(); ++i) {
        descriptor_set_layouts[i] = descriptor_set_layouts[0];
    }
    VkDescriptorSetAllocateInfo descriptor_set_allocate_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .descriptorPool = descriptor_pool,
        .descriptorSetCount = (uint32_t)image_views.size(),
        .pSetLayouts = descriptor_set_layouts.data()};
    std::vector<VkDescriptorSet> descriptor_sets{image_views.size()};
    if (vkAllocateDescriptorSets(device.device, &descriptor_set_allocate_info,
                                 descriptor_sets.data()) != VK_SUCCESS) {
        std::cerr << "failed to allocate descriptor sets"
                  << "\n";
        return 1;
    }
    std::vector<VkDescriptorImageInfo> descriptor_image_info{
        image_views.size()};
    std::vector<VkWriteDescriptorSet> write_decriptor_sets{image_views.size()};
    for (auto i = 0; i < descriptor_set_layouts.size(); ++i) {
        descriptor_image_info[i] = {.imageView = image_views[i],
                                    .imageLayout = VK_IMAGE_LAYOUT_GENERAL};
        write_decriptor_sets[i] = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = VK_NULL_HANDLE,
            .dstSet = descriptor_sets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .pImageInfo = &descriptor_image_info[i],
            .pBufferInfo = VK_NULL_HANDLE,
            .pTexelBufferView = VK_NULL_HANDLE};
    }
    vkUpdateDescriptorSets(device.device, image_views.size(),
                           write_decriptor_sets.data(), 0, VK_NULL_HANDLE);
    std::ifstream file("src/lib.spv", std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "failed to open file"
                  << "\n";
        return 1;
    }
    size_t file_size = (size_t)file.tellg();
    std::vector<char> code(file_size);
    file.seekg(0);
    file.read(code.data(), static_cast<std::streamsize>(file_size));
    file.close();
    VkShaderModuleCreateInfo shader_module_create_info = {};
    shader_module_create_info.sType =
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = code.size();
    shader_module_create_info.pCode =
        reinterpret_cast<const uint32_t *>(code.data());
    VkShaderModule shader_module;
    if (vkCreateShaderModule(device.device, &shader_module_create_info,
                             VK_NULL_HANDLE, &shader_module) != VK_SUCCESS) {
        std::cerr << "failed to create shader module"
                  << "\n";
        return 1;
    }
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = VK_NULL_HANDLE,
        .setLayoutCount = 1,
        .pSetLayouts = descriptor_set_layouts.data(),
        .pushConstantRangeCount = VK_NULL_HANDLE,
        .pPushConstantRanges = VK_NULL_HANDLE};
    VkPipelineLayout pipeline_layout;
    vkCreatePipelineLayout(device.device, &pipeline_layout_create_info,
                           VK_NULL_HANDLE, &pipeline_layout);
    VkSpecializationMapEntry specialization_map_entries[3]{
        {.constantID = 0,
         .offset = sizeof(uint32_t) * 0,
         .size = sizeof(uint32_t)},
        {.constantID = 1,
         .offset = sizeof(uint32_t) * 1,
         .size = sizeof(uint32_t)},
        {.constantID = 2,
         .offset = sizeof(uint32_t) * 2,
         .size = sizeof(uint32_t)}};
    uint32_t specialization_data[3] = {16, 16, 1};
    VkSpecializationInfo specialization_info{.mapEntryCount = 3,
                                             .pMapEntries =
                                                 specialization_map_entries,
                                             .dataSize = sizeof(uint32_t) * 3,
                                             .pData = specialization_data};
    VkComputePipelineCreateInfo compute_pipeline_create_info{
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .stage = {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                  .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                  .module = shader_module,
                  .pName = "device_kernel",
                  .pSpecializationInfo = &specialization_info},
        .layout = pipeline_layout,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = VK_NULL_HANDLE,
    };
    VkPipeline pipeline;
    if (vkCreateComputePipelines(device.device, VK_NULL_HANDLE, 1,
                                 &compute_pipeline_create_info, VK_NULL_HANDLE,
                                 &pipeline) != VK_SUCCESS) {
        std::cerr << "failed to create compute pipelines"
                  << "\n";
        return 1;
    }
    VkQueue queue;
    if (auto result = device.get_queue(vkb::QueueType::present);
        !result.has_value()) {
        std::cerr << "failed to get queue: " << result.error().message()
                  << "\n";
        return 1;
    } else {
        queue = result.value();
    }
    auto queue_family_index =
        device.get_queue_index(vkb::QueueType::present).value();
    VkCommandPoolCreateInfo command_pool_create_info = {};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex = queue_family_index;
    VkCommandPool command_pool;
    if (vkCreateCommandPool(device.device, &command_pool_create_info, nullptr,
                            &command_pool) != VK_SUCCESS) {
        std::cerr << "failed to create command pool\n";
        return 1;
    }
    std::vector<VkCommandBuffer> command_buffers{image_views.size()};
    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
    command_buffer_allocate_info.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount =
        (uint32_t)command_buffers.size();

    if (vkAllocateCommandBuffers(device.device, &command_buffer_allocate_info,
                                 command_buffers.data()) != VK_SUCCESS) {
        std::cerr << "failed to allocate command buffers\n";
        return 1;
    }

    int w, h;
    SDL_Vulkan_GetDrawableSize(window, &w, &h);
    for (size_t i = 0; i < command_buffers.size(); i++) {
        VkCommandBufferBeginInfo command_buffer_begin_info = {};
        command_buffer_begin_info.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(command_buffers[i],
                                 &command_buffer_begin_info) != VK_SUCCESS) {
            std::cerr << "failed to begin command buffer\n";
            return 1;
        }
        vkCmdBindDescriptorSets(
            command_buffers[i], VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout,
            0, 1, &descriptor_sets[i], VK_NULL_HANDLE, VK_NULL_HANDLE);
        vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_COMPUTE,
                          pipeline);
        VkImageMemoryBarrier begin_image_memory_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = VK_NULL_HANDLE,
            .srcAccessMask = VK_NULL_HANDLE,
            .dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .srcQueueFamilyIndex = queue_family_index,
            .dstQueueFamilyIndex = queue_family_index,
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
        vkCmdDispatch(command_buffers[i], w / specialization_data[0],
                      h / specialization_data[1], 1);
        VkImageMemoryBarrier end_image_memory_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = VK_NULL_HANDLE,
            .srcAccessMask = VK_NULL_HANDLE,
            .dstAccessMask = VK_NULL_HANDLE,
            .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = queue_family_index,
            .dstQueueFamilyIndex = queue_family_index,
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
            std::cerr << "failed to end command buffer\n";
            return 1;
        }
    }

    std::vector<VkSemaphore> available_semaphores{image_views.size()};
    std::vector<VkSemaphore> finished_semaphores{image_views.size()};
    std::vector<VkFence> in_flight_fences{image_views.size()};
    std::vector<VkFence> image_in_flight{image_views.size(), VK_NULL_HANDLE};

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (auto i = 0; i < image_views.size(); i++) {
        if (vkCreateSemaphore(device.device, &semaphore_info, nullptr,
                              &available_semaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device.device, &semaphore_info, nullptr,
                              &finished_semaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device.device, &fence_info, nullptr,
                          &in_flight_fences[i]) != VK_SUCCESS) {
            std::cerr << "failed to create sync objects\n";
            return 1;
        }
    }
    SDL_Event event;
    int quit = 0;
    int index = 0;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYUP:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    quit = 1;
                break;

            case SDL_QUIT:
                quit = 1;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE &&
                    event.window.windowID == SDL_GetWindowID(window))
                    quit = 1;
                break;

            default:
                break;
            }
        }
        vkWaitForFences(device.device, 1, &in_flight_fences[index], VK_TRUE,
                        UINT64_MAX);

        uint32_t image_index = 0;
        VkResult result = vkAcquireNextImageKHR(
            device.device, swapchain.swapchain, UINT64_MAX,
            available_semaphores[index], VK_NULL_HANDLE, &image_index);

        if (image_in_flight[image_index] != VK_NULL_HANDLE) {
            vkWaitForFences(device.device, 1, &image_in_flight[image_index],
                            VK_TRUE, UINT64_MAX);
        }
        image_in_flight[image_index] = in_flight_fences[index];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore wait_semaphores[] = {available_semaphores[index]};
        VkPipelineStageFlags wait_stages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = wait_semaphores;
        submitInfo.pWaitDstStageMask = wait_stages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &command_buffers[image_index];

        VkSemaphore signal_semaphores[] = {finished_semaphores[index]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signal_semaphores;

        vkResetFences(device.device, 1, &in_flight_fences[index]);

        if (vkQueueSubmit(queue, 1, &submitInfo, in_flight_fences[index]) !=
            VK_SUCCESS) {
            std::cerr << "failed to submit draw command buffer\n";
            return -1; //"failed to submit draw command buffer
        }

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signal_semaphores;

        VkSwapchainKHR swapChains[] = {swapchain.swapchain};
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swapChains;

        present_info.pImageIndices = &image_index;

        result = vkQueuePresentKHR(queue, &present_info);

        index = (index + 1) % image_views.size();
    }
    SDL_Quit();
    return 0;
}
