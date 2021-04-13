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
    VmaAllocator allocator;
    if (auto error = create_device_allocator(instance, surface, physical_device,
                                             device, allocator)) {
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
    VkDescriptorPool descriptor_pool;
    if (auto error = create_descriptor_pool(device, descriptor_pool)) {
        return -1;
    }
    VkDescriptorSetLayout set_layout;
    VkPipelineLayout pipeline_layout;
    if (auto error =
            create_set_pipeline_layout(device, set_layout, pipeline_layout)) {
        return -1;
    }
    auto module_name = "main.hpp";
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
    std::vector<VkFramebuffer> framebuffers;
    VkRenderPass render_pass;
    if (auto error =
            create_swapchain_semaphores_fences_render_pass_framebuffers(
                device, swapchain, images, image_views, signal_fences,
                wait_semaphores, signal_semaphores, render_pass,
                framebuffers)) {
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
    ImGuiIO imgui_io;
    if (auto error =
            imgui_initialize(window, instance, physical_device, device, queue,
                             queue_index, descriptor_pool, swapchain,
                             render_pass, command_buffers[0], imgui_io)) {
        return -1;
    }
    uint32_t index = 0;
    uint32_t quit = 0;
    SDL_Event event;
    bool show_demo_window = true;
    auto reset = [&]() -> std::optional<int> {
        vkDeviceWaitIdle(device.device);
        vkFreeCommandBuffers(device.device, command_pool,
                             command_buffers.size(), command_buffers.data());
        vkFreeDescriptorSets(device.device, descriptor_pool,
                             descriptor_sets.size(), descriptor_sets.data());
        if (auto error =
                create_swapchain_semaphores_fences_render_pass_framebuffers(
                    device, swapchain, images, image_views, signal_fences,
                    wait_semaphores, signal_semaphores, render_pass,
                    framebuffers, true)) {
            return -1;
        }
        if (auto error = allocate_descriptor_sets(
                device, swapchain, image_views, set_layout, descriptor_pool,
                descriptor_sets)) {
            return -1;
        }
        if (auto error = allocate_command_buffers(
                window, device, queue_index, command_pool, pipeline_layout,
                pipeline, swapchain, images, image_views, descriptor_sets,
                command_buffers)) {
            return -1;
        }
        ImGui_ImplVulkan_SetMinImageCount(swapchain.image_count);
        index = 0;
        return {};
    };
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
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
                    if (auto error = reset()) {
                        return -1;
                    }
                }
                break;

            default:
                break;
            }
        }
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
        ImGui::Render();
        ImDrawData *draw_data = ImGui::GetDrawData();
        if (auto error = queue_submit(
                device, queue, swapchain, signal_fences, wait_semaphores,
                signal_semaphores, command_buffers, index,
                [&](const uint32_t &index,
                    const VkCommandBuffer &command_buffer)
                    -> std::optional<int> {
                    int width, height;
                    SDL_Vulkan_GetDrawableSize(window, &width, &height);
                    vkCmdBindPipeline(command_buffer,
                                      VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
                    vkCmdBindDescriptorSets(
                        command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                        pipeline_layout, 0, 1, &descriptor_sets[index],
                        VK_NULL_HANDLE, VK_NULL_HANDLE);
                    VkImageMemoryBarrier begin_image_memory_barrier{
                        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                        .pNext = VK_NULL_HANDLE,
                        .srcAccessMask = VK_NULL_HANDLE,
                        .dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
                        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
                        .srcQueueFamilyIndex = queue_index,
                        .dstQueueFamilyIndex = queue_index,
                        .image = images[index],
                        .subresourceRange = {.aspectMask =
                                                 VK_IMAGE_ASPECT_COLOR_BIT,
                                             .baseMipLevel = VK_NULL_HANDLE,
                                             .levelCount = 1,
                                             .baseArrayLayer = VK_NULL_HANDLE,
                                             .layerCount = 1}};
                    vkCmdPipelineBarrier(
                        command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_NULL_HANDLE,
                        VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,
                        VK_NULL_HANDLE, 1, &begin_image_memory_barrier);
                    vkCmdDispatch(command_buffer, width / GROUP_SIZE + 1,
                                  height / GROUP_SIZE + 1, 1);
                    VkImageMemoryBarrier end_image_memory_barrier{
                        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                        .pNext = VK_NULL_HANDLE,
                        .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
                        .dstAccessMask =
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
                        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                        .srcQueueFamilyIndex = queue_index,
                        .dstQueueFamilyIndex = queue_index,
                        .image = images[index],
                        .subresourceRange = {.aspectMask =
                                                 VK_IMAGE_ASPECT_COLOR_BIT,
                                             .baseMipLevel = VK_NULL_HANDLE,
                                             .levelCount = 1,
                                             .baseArrayLayer = VK_NULL_HANDLE,
                                             .layerCount = 1}};
                    vkCmdPipelineBarrier(
                        command_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_NULL_HANDLE,
                        VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,
                        VK_NULL_HANDLE, 1, &end_image_memory_barrier);
                    VkViewport viewport = {
                        .x = 0.0f,
                        .y = 0.0f,
                        .width = (float)swapchain.extent.width,
                        .height = (float)swapchain.extent.height,
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f};
                    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
                    VkRect2D scissor = {.offset = {0, 0},
                                        .extent = swapchain.extent};
                    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
                    VkClearValue clear_values{{{0.0f, 0.0f, 0.0f, 1.0f}}};
                    VkRenderPassBeginInfo begin_info = {
                        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                        .renderPass = render_pass,
                        .framebuffer = framebuffers[index],
                        .renderArea = {.offset = {0, 0},
                                       .extent = swapchain.extent},
                        .clearValueCount = 1,
                        .pClearValues = &clear_values};
                    vkCmdBeginRenderPass(command_buffer, &begin_info,
                                         VK_SUBPASS_CONTENTS_INLINE);
                    ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer);
                    vkCmdEndRenderPass(command_buffer);
                    return {};
                })) {
            if (error == 0) {
                if (auto error = reset()) {
                    return -1;
                }
            }
            if (error == 1) {
            } else {
                return -1;
            }
        }
    }
    vkDeviceWaitIdle(device.device);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    vkFreeCommandBuffers(device.device, command_pool, command_buffers.size(),
                         command_buffers.data());
    vkFreeDescriptorSets(device.device, descriptor_pool, descriptor_sets.size(),
                         descriptor_sets.data());
    for (auto &framebuffer : framebuffers) {
        vkDestroyFramebuffer(device.device, framebuffer, VK_NULL_HANDLE);
    }
    vkDestroyRenderPass(device.device, render_pass, VK_NULL_HANDLE);
    for (auto &semaphore : signal_semaphores) {
        vkDestroySemaphore(device.device, semaphore, VK_NULL_HANDLE);
    }
    for (auto &semaphore : wait_semaphores) {
        vkDestroySemaphore(device.device, semaphore, VK_NULL_HANDLE);
    }
    for (auto &fence : signal_fences) {
        vkDestroyFence(device.device, fence, VK_NULL_HANDLE);
    }
    swapchain.destroy_image_views(image_views);
    vkb::destroy_swapchain(swapchain);
    vkDestroyPipeline(device.device, pipeline, VK_NULL_HANDLE);
    vkDestroyShaderModule(device.device, shader_module, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(device.device, pipeline_layout, VK_NULL_HANDLE);
    vkDestroyDescriptorSetLayout(device.device, set_layout, VK_NULL_HANDLE);
    vkDestroyDescriptorPool(device.device, descriptor_pool, VK_NULL_HANDLE);
    vkDestroyCommandPool(device.device, command_pool, VK_NULL_HANDLE);
    vmaDestroyAllocator(allocator);
    vkb::destroy_device(device);
    vkDestroySurfaceKHR(instance.instance, surface, VK_NULL_HANDLE);
    vkb::destroy_instance(instance);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
