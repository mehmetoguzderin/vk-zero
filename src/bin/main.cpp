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
    std::vector<VkFence> signal_fences;
    std::vector<VkSemaphore> wait_semaphores, signal_semaphores;
    std::vector<VkFence> wait_fences;
    if (auto error = create_swapchain_semaphores_fences(
            device, swapchain, images, image_views, signal_fences,
            wait_semaphores, signal_semaphores, wait_fences)) {
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
    int index = 0;
    int quit = 0;
    SDL_Event event;
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
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    vkDeviceWaitIdle(device.device);
                    vkFreeCommandBuffers(device.device, command_pool,
                                         command_buffers.size(),
                                         command_buffers.data());
                    vkDestroyDescriptorPool(device.device, descriptor_pool,
                                            VK_NULL_HANDLE);
                    if (auto error = create_swapchain_semaphores_fences(
                            device, swapchain, images, image_views,
                            signal_fences, wait_semaphores, signal_semaphores,
                            wait_fences, true)) {
                        return -1;
                    }
                    if (auto error = create_descriptor_pool(device, swapchain,
                                                            descriptor_pool)) {
                        return -1;
                    }
                    if (auto error = allocate_descriptor_sets(
                            device, swapchain, image_views, set_layout,
                            descriptor_pool, descriptor_sets)) {
                        return -1;
                    }
                    if (auto error = allocate_command_buffers(
                            window, device, queue_index, command_pool,
                            pipeline_layout, pipeline, swapchain, images,
                            image_views, descriptor_sets, command_buffers)) {
                        return -1;
                    }
                    index = 0;
                }
                break;

            default:
                break;
            }
        }
        if (auto error = queue_submit(device, queue, swapchain, signal_fences,
                                      wait_semaphores, signal_semaphores,
                                      command_buffers, index, wait_fences)) {
            if (error == 0) {
                vkDeviceWaitIdle(device.device);
                vkFreeCommandBuffers(device.device, command_pool,
                                     command_buffers.size(),
                                     command_buffers.data());
                vkDestroyDescriptorPool(device.device, descriptor_pool,
                                        VK_NULL_HANDLE);
                if (auto error = create_swapchain_semaphores_fences(
                        device, swapchain, images, image_views, signal_fences,
                        wait_semaphores, signal_semaphores, wait_fences,
                        true)) {
                    return -1;
                }
                if (auto error = create_descriptor_pool(device, swapchain,
                                                        descriptor_pool)) {
                    return -1;
                }
                if (auto error = allocate_descriptor_sets(
                        device, swapchain, image_views, set_layout,
                        descriptor_pool, descriptor_sets)) {
                    return -1;
                }
                if (auto error = allocate_command_buffers(
                        window, device, queue_index, command_pool,
                        pipeline_layout, pipeline, swapchain, images,
                        image_views, descriptor_sets, command_buffers)) {
                    return -1;
                }
                index = 0;
            } else {
                return -1;
            }
        } else {
            index = (index + 1) % swapchain.image_count;
        }
    }
    SDL_Quit();
    return 0;
}
