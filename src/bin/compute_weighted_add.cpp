#include "compute_weighted_add.h"

int main(int argc, char *argv[]) {
    if (auto error = initialize()) {
        return -1;
    }
    auto create_name = "compute_weighted_add";
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
    uint64_t length = 16384;
    VkBuffer buffer_storage_a;
    VmaAllocation allocation_storage_a;
    VmaAllocationInfo allocation_info_storage_a;
    if (auto error = compute_weighted_add::create_buffer_storage(
            allocator, length * ELEMENT_SIZE, buffer_storage_a,
            allocation_storage_a, allocation_info_storage_a)) {
        return -1;
    }
    auto pointer_a = (float4 *)allocation_info_storage_a.pMappedData;
    VkBuffer buffer_storage_b;
    VmaAllocation allocation_storage_b;
    VmaAllocationInfo allocation_info_storage_b;
    if (auto error = compute_weighted_add::create_buffer_storage(
            allocator, length * ELEMENT_SIZE, buffer_storage_b,
            allocation_storage_b, allocation_info_storage_b)) {
        return -1;
    }
    auto pointer_b = (float4 *)allocation_info_storage_b.pMappedData;
    pointer_b[4094] = vec4(.5f);
    VkBuffer buffer_storage_c;
    VmaAllocation allocation_storage_c;
    VmaAllocationInfo allocation_info_storage_c;
    if (auto error = compute_weighted_add::create_buffer_storage(
            allocator, length * ELEMENT_SIZE, buffer_storage_c,
            allocation_storage_c, allocation_info_storage_c)) {
        return -1;
    }
    auto pointer_c = (float4 *)allocation_info_storage_c.pMappedData;
    VkBuffer buffer_storage_d;
    VmaAllocation allocation_storage_d;
    VmaAllocationInfo allocation_info_storage_d;
    if (auto error = compute_weighted_add::create_buffer_storage(
            allocator, length * ELEMENT_SIZE, buffer_storage_d,
            allocation_storage_d, allocation_info_storage_d)) {
        return -1;
    }
    auto pointer_d = (float4 *)allocation_info_storage_d.pMappedData;
    ComputeWeightedAddConstants constants{
        .weights = vec4(1.f, 1.f, 1.f, 1.f),
        .length = uvec2(length / ELEMENT_WIDTH, length % ELEMENT_WIDTH)};
    if (constants.length.y > 0) {
        constants.length.x += 1;
    }
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
    if (auto error = compute_weighted_add::create_set_pipeline_layout(
            device, set_layout, pipeline_layout)) {
        return -1;
    }
    uint3 local_size = uvec3(16, 32, 1);
    auto module_name = "compute_weighted_add.hpp";
    VkShaderModule shader_module;
    if (auto error = create_shader_module(device, module_name, shader_module)) {
        return -1;
    }
    auto entry_name = "compute_weighted_add_kernel";
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
    if (auto error = compute_weighted_add::allocate_descriptor_sets(
            device, buffer_storage_a, allocation_info_storage_a,
            buffer_storage_b, allocation_info_storage_b, buffer_storage_c,
            allocation_info_storage_c, buffer_storage_d,
            allocation_info_storage_d, buffer_uniform, allocation_info_uniform,
            swapchain, image_views, set_layout, descriptor_pool,
            descriptor_sets)) {
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
    uint32_t image_index = 0;
    if (vkWaitForFences(device.device, 1, &signal_fences[image_index], VK_TRUE,
                        UINT64_MAX) != VK_SUCCESS) {
        return -1;
    }
    if (vkResetFences(device.device, 1, &signal_fences[image_index]) !=
        VK_SUCCESS) {
        return -1;
    }
    if (vkResetCommandBuffer(compute_command_buffers[image_index],
                             VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT) !=
        VK_SUCCESS) {
        return -1;
    }
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
    if (vkBeginCommandBuffer(compute_command_buffers[image_index],
                             &begin_info) != VK_SUCCESS) {
        return -1;
    }
    vkCmdBindPipeline(compute_command_buffers[image_index],
                      VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(compute_command_buffers[image_index],
                            VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0,
                            1, &descriptor_sets[image_index], 0, nullptr);
    vkCmdDispatch(compute_command_buffers[image_index],
                  length / (local_size.x * local_size.y) + 1, 1, 1);
    if (vkEndCommandBuffer(compute_command_buffers[image_index]) !=
        VK_SUCCESS) {
        return -1;
    }
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
    VkSubmitInfo submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                .waitSemaphoreCount = 0,
                                .pWaitSemaphores = nullptr,
                                .pWaitDstStageMask = wait_stages,
                                .commandBufferCount = 1,
                                .pCommandBuffers =
                                    &compute_command_buffers[image_index],
                                .signalSemaphoreCount = 0,
                                .pSignalSemaphores = nullptr};
    if (vkQueueSubmit(compute_queue, 1, &submit_info,
                      signal_fences[image_index]) != VK_SUCCESS) {
        return -1;
    }
    vkDeviceWaitIdle(device.device);
    auto &element = pointer_a[4094];
    std::cout << element.x << " " << element.y << " " << element.z << " "
              << element.w << "\n";
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
    vmaDestroyBuffer(allocator, buffer_storage_d, allocation_storage_d);
    vmaDestroyBuffer(allocator, buffer_storage_c, allocation_storage_c);
    vmaDestroyBuffer(allocator, buffer_storage_b, allocation_storage_b);
    vmaDestroyBuffer(allocator, buffer_storage_a, allocation_storage_a);
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
