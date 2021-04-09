#include "main.h"

int main(int argc, char *argv[]) {
    if (auto error = initialize()) {
        return -1;
    }
    auto create_name = "main";
    SDL_Window *window;
    vkb::Instance instance;
    VkSurfaceKHR surface;
    if (auto error = create_window_instance_surface(create_name, window,
                                                    instance, surface)) {
        return -1;
    }
    vkb::PhysicalDevice physical_device;
    vkb::Device device;
    if (auto error =
            create_device(instance, surface, physical_device, device)) {
        return -1;
    }
    VkQueue queue;
    uint32_t queue_index;
    if (auto error = get_queue(device, queue, queue_index)) {
        return -1;
    }
    VkCommandPool command_pool;
    if (auto error = create_command_pool(device, queue_index, command_pool)) {
        return -1;
    }
    VkDescriptorSetLayout set_layout;
    VkPipelineLayout pipeline_layout;
    if (auto error =
            create_set_pipeline_layout(device, set_layout, pipeline_layout)) {
        return -1;
    }
    auto module_name = "src/lib.spv";
    VkShaderModule shader_module;
    if (auto error = create_shader_module(device, module_name, shader_module)) {
        return -1;
    }
    auto entry_name = "device_kernel";
    VkPipeline pipeline;
    if (auto error = create_pipeline(device, pipeline_layout, shader_module,
                                     entry_name, pipeline)) {
        return -1;
    }
    vkb::Swapchain swapchain;
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
    std::vector<VkFence> sf;
    std::vector<VkSemaphore> ws, ss;
    std::vector<VkFence> wf;
    if (auto error = create_swapchain_semaphores_fences(
            device, swapchain, images, image_views, sf, ws, ss, wf)) {
        return -1;
    }
    VkDescriptorPool descriptor_pool;
    if (auto error =
            create_descriptor_pool(device, swapchain, descriptor_pool)) {
        return -1;
    }
    std::vector<VkDescriptorSet> descriptor_sets;
    if (auto error =
            allocate_descriptor_sets(device, swapchain, image_views, set_layout,
                                     descriptor_pool, descriptor_sets)) {
        return -1;
    }
    std::vector<VkCommandBuffer> command_buffers;
    if (auto error = allocate_command_buffers(
            window, device, queue_index, command_pool, pipeline_layout,
            pipeline, swapchain, images, image_views, descriptor_sets,
            command_buffers)) {
        return -1;
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
