#include "main.h"

int main(int argc, char *argv[]) {
    if (auto error = initialize()) {
        return -1;
    }
    auto create_name = "main";
    GLFWwindow *window;
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
    VkQueue graphics_queue, compute_queue;
    uint32_t graphics_queue_index, compute_queue_index;
    if (auto error = get_graphics_compute_queue(
            device, graphics_queue, graphics_queue_index, compute_queue,
            compute_queue_index)) {
        return -1;
    }
    VkCommandPool graphics_command_pool;
    if (auto error = create_command_pool(device, graphics_queue_index,
                                         graphics_command_pool)) {
        return -1;
    }
    VkCommandPool compute_command_pool;
    if (auto error = create_command_pool(device, compute_queue_index,
                                         compute_command_pool)) {
        return -1;
    }
    VkDescriptorPool descriptor_pool;
    if (auto error = create_descriptor_pool(device, descriptor_pool)) {
        return -1;
    }
    MainConstants constants{.color = vec4(1.f, 1.f, 1.f, 1.f)};
    VkBuffer buffer_uniform;
    VmaAllocation allocation_uniform;
    VmaAllocationInfo allocation_info_uniform;
    if (auto error = create_buffer_uniform(allocator, sizeof(constants),
                                           buffer_uniform, allocation_uniform,
                                           allocation_info_uniform)) {
        return -1;
    }
    memcpy(allocation_info_uniform.pMappedData, &constants, sizeof(constants));
    VkDescriptorSetLayout set_layout;
    VkPipelineLayout pipeline_layout;
    if (auto error =
            create_set_pipeline_layout(device, set_layout, pipeline_layout)) {
        return -1;
    }
    uint3 local_size = uvec3(16, 16, 1);
    auto module_name = "main.hpp";
    VkShaderModule shader_module;
    if (auto error = create_shader_module(device, module_name, shader_module)) {
        return -1;
    }
    auto entry_name = "device_kernel";
    VkPipeline pipeline;
    if (auto error = create_pipeline(device, pipeline_layout, shader_module,
                                     local_size, entry_name, pipeline)) {
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
    if (auto error = allocate_descriptor_sets(
            device, buffer_uniform, allocation_info_uniform, swapchain,
            image_views, set_layout, descriptor_pool, descriptor_sets)) {
        return -1;
    }
    std::vector<VkCommandBuffer> graphics_command_buffers;
    if (auto error = allocate_command_buffers(
            window, device, graphics_queue_index, graphics_command_pool,
            swapchain, graphics_command_buffers)) {
        return -1;
    }
    std::vector<VkCommandBuffer> compute_command_buffers;
    if (auto error = allocate_command_buffers(
            window, device, compute_queue_index, compute_command_pool,
            swapchain, compute_command_buffers)) {
        return -1;
    }
    ImGuiIO imgui_io;
    if (auto error = imgui_initialize(window, instance, physical_device, device,
                                      graphics_queue, graphics_queue_index,
                                      descriptor_pool, swapchain, render_pass,
                                      graphics_command_buffers[0], imgui_io)) {
        return -1;
    }
    uint32_t index = 0;
    uint32_t quit = 0;
    bool show_demo_window = true;
    static GLFWkeyfun key_function = [](GLFWwindow *window, int key,
                                        int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    };
    glfwSetKeyCallback(window, key_function);
    uint32_t resize = 0;
    glfwSetWindowUserPointer(window, &resize);
    static GLFWframebuffersizefun size_function = [](GLFWwindow *window,
                                                     int width, int height) {
        auto resize = static_cast<uint32_t *>(glfwGetWindowUserPointer(window));
        *resize = 1;
    };
    glfwSetFramebufferSizeCallback(window, size_function);
    auto reset = [&]() -> std::optional<int> {
        vkDeviceWaitIdle(device.device);
        vkFreeCommandBuffers(device.device, compute_command_pool,
                             compute_command_buffers.size(),
                             compute_command_buffers.data());
        vkFreeCommandBuffers(device.device, graphics_command_pool,
                             graphics_command_buffers.size(),
                             graphics_command_buffers.data());
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
                device, buffer_uniform, allocation_info_uniform, swapchain,
                image_views, set_layout, descriptor_pool, descriptor_sets)) {
            return -1;
        }
        if (auto error = allocate_command_buffers(
                window, device, graphics_queue_index, graphics_command_pool,
                swapchain, graphics_command_buffers)) {
            return -1;
        }
        if (auto error = allocate_command_buffers(
                window, device, compute_queue_index, compute_command_pool,
                swapchain, compute_command_buffers)) {
            return -1;
        }
        ImGui_ImplVulkan_SetMinImageCount(swapchain.image_count);
        index = 0;
        return {};
    };
    while (!quit && !glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (resize) {
            if (auto error = reset()) {
                return -1;
            }
            resize = 0;
        }
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
        ImGui::Render();
        ImDrawData *draw_data = ImGui::GetDrawData();
        if (auto error = frame_submit(
                device, graphics_queue, compute_queue, swapchain, signal_fences,
                wait_semaphores, signal_semaphores, graphics_command_buffers,
                compute_command_buffers, index,
                [&](const uint32_t &index,
                    const VkCommandBuffer &command_buffer)
                    -> std::optional<int> {
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
                    VkClearValue clear_values{{{0.0f, 0.0f, 0.0f, 0.0f}}};
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
                    vkCmdBindPipeline(command_buffer,
                                      VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
                    vkCmdBindDescriptorSets(
                        command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                        pipeline_layout, 0, 1, &descriptor_sets[index], 0,
                        nullptr);
                    VkImageMemoryBarrier image_memory_barrier{
                        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                        .pNext = nullptr,
                        .srcAccessMask = 0,
                        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT |
                                         VK_ACCESS_SHADER_WRITE_BIT,
                        .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
                        .newLayout = VK_IMAGE_LAYOUT_GENERAL,
                        .srcQueueFamilyIndex = graphics_queue_index,
                        .dstQueueFamilyIndex = compute_queue_index,
                        .image = images[index],
                        .subresourceRange = {.aspectMask =
                                                 VK_IMAGE_ASPECT_COLOR_BIT,
                                             .baseMipLevel = 0,
                                             .levelCount = 1,
                                             .baseArrayLayer = 0,
                                             .layerCount = 1}};
                    vkCmdPipelineBarrier(
                        command_buffer,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0,
                        nullptr, 1, &image_memory_barrier);
                    return {};
                },
                [&](const uint32_t &index,
                    const VkCommandBuffer &command_buffer)
                    -> std::optional<int> {
                    vkCmdBindPipeline(command_buffer,
                                      VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
                    vkCmdBindDescriptorSets(
                        command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                        pipeline_layout, 0, 1, &descriptor_sets[index], 0,
                        nullptr);
                    vkCmdDispatch(command_buffer, width / local_size.x + 1,
                                  height / local_size.y + 1, 1);
                    VkImageMemoryBarrier image_memory_barrier{
                        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                        .pNext = nullptr,
                        .srcAccessMask = VK_ACCESS_SHADER_READ_BIT |
                                         VK_ACCESS_SHADER_WRITE_BIT,
                        .dstAccessMask = 0,
                        .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
                        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                        .srcQueueFamilyIndex = compute_queue_index,
                        .dstQueueFamilyIndex = graphics_queue_index,
                        .image = images[index],
                        .subresourceRange = {.aspectMask =
                                                 VK_IMAGE_ASPECT_COLOR_BIT,
                                             .baseMipLevel = 0,
                                             .levelCount = 1,
                                             .baseArrayLayer = 0,
                                             .layerCount = 1}};
                    vkCmdPipelineBarrier(
                        command_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
                        nullptr, 1, &image_memory_barrier);
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
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkFreeCommandBuffers(device.device, compute_command_pool,
                         compute_command_buffers.size(),
                         compute_command_buffers.data());
    vkFreeCommandBuffers(device.device, graphics_command_pool,
                         graphics_command_buffers.size(),
                         graphics_command_buffers.data());
    vkFreeDescriptorSets(device.device, descriptor_pool, descriptor_sets.size(),
                         descriptor_sets.data());
    for (auto &framebuffer : framebuffers) {
        vkDestroyFramebuffer(device.device, framebuffer, nullptr);
    }
    vkDestroyRenderPass(device.device, render_pass, nullptr);
    for (auto &semaphore : signal_semaphores) {
        vkDestroySemaphore(device.device, semaphore, nullptr);
    }
    for (auto &semaphore : wait_semaphores) {
        vkDestroySemaphore(device.device, semaphore, nullptr);
    }
    for (auto &fence : signal_fences) {
        vkDestroyFence(device.device, fence, nullptr);
    }
    swapchain.destroy_image_views(image_views);
    vkb::destroy_swapchain(swapchain);
    vkDestroyPipeline(device.device, pipeline, nullptr);
    vkDestroyShaderModule(device.device, shader_module, nullptr);
    vkDestroyPipelineLayout(device.device, pipeline_layout, nullptr);
    vkDestroyDescriptorSetLayout(device.device, set_layout, nullptr);
    vmaDestroyBuffer(allocator, buffer_uniform, allocation_uniform);
    vkDestroyDescriptorPool(device.device, descriptor_pool, nullptr);
    vkDestroyCommandPool(device.device, compute_command_pool, nullptr);
    vkDestroyCommandPool(device.device, graphics_command_pool, nullptr);
    vmaDestroyAllocator(allocator);
    vkb::destroy_device(device);
    vkDestroySurfaceKHR(instance.instance, surface, nullptr);
    vkb::destroy_instance(instance);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
