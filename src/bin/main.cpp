#include "lib.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
    if (auto result = volkInitialize(); result != VK_SUCCESS) {
        std::cerr << "Failed to initialize Volk. Error: " << result << "\n";
        return 1;
    }
    if (auto result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS |
                               SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER);
        result < 0) {
        std::cerr << "Failed to initialize SDL. Error: " << SDL_GetError()
                  << "\n";
        return 1;
    }
    auto app_name = "main";
    auto window = SDL_CreateWindow(
        app_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    uint32_t extensions_count = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extensions_count, NULL);
    std::vector<const char *> extensions{extensions_count};
    SDL_Vulkan_GetInstanceExtensions(window, &extensions_count,
                                     extensions.data());
    vkb::InstanceBuilder instance_builder{};
    for (auto &extension : extensions) {
        instance_builder.enable_extension(extension);
    }
    vkb::Instance instance;
    if (auto result = instance_builder.set_app_name(app_name)
                          .require_api_version(1, 1)
                          .use_default_debug_messenger()
                          .request_validation_layers()
                          .build();
        !result) {
        std::cerr << "Failed to create Vulkan instance. Error: "
                  << result.error().message() << "\n";
        return 1;
    } else {
        instance = result.value();
    }
    volkLoadInstance(instance.instance);
    VkSurfaceKHR surface;
    if (auto result =
            SDL_Vulkan_CreateSurface(window, instance.instance, &surface);
        !result) {
        std::cerr << "Failed to create surface."
                  << "\n";
        return 1;
    }
    vkb::PhysicalDevice physical_device;
    if (auto result = vkb::PhysicalDeviceSelector{instance}
                          .set_minimum_version(1, 1)
                          .set_surface(surface)
                          .select();
        !result) {
        std::cerr << result.error().message() << "\n";
        return 1;
    } else {
        physical_device = result.value();
    }
    vkb::Device device;
    if (auto result = vkb::DeviceBuilder{physical_device}.build(); !result) {
        std::cerr << result.error().message() << "\n";
        return 1;
    } else {
        device = result.value();
    }
    volkLoadDevice(device.device);
    vkb::Swapchain swapchain;
    if (auto result =
            vkb::SwapchainBuilder{device}
                .add_format_feature_flags(VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
                .add_image_usage_flags(VK_IMAGE_USAGE_STORAGE_BIT)
                .build();
        !result) {
        std::cerr << result.error().message() << " " << result.vk_result()
                  << "\n";
        return 1;
    } else {
        swapchain = result.value();
    }
    auto swapchain_images = swapchain.get_images().value();
    auto swapchain_image_views = swapchain.get_image_views().value();
    VkDescriptorPoolSize descriptor_pool_size{
        .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = (uint32_t)swapchain_image_views.size()};
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
        swapchain_image_views.size()};
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
        .descriptorSetCount = (uint32_t)swapchain_image_views.size(),
        .pSetLayouts = descriptor_set_layouts.data()};
    std::vector<VkDescriptorSet> descriptor_sets{swapchain_image_views.size()};
    if (vkAllocateDescriptorSets(device.device, &descriptor_set_allocate_info,
                                 descriptor_sets.data()) != VK_SUCCESS) {
        std::cerr << "failed to allocate descriptor sets"
                  << "\n";
        return 1;
    }
    std::vector<VkDescriptorImageInfo> descriptor_image_info{
        swapchain_image_views.size()};
    std::vector<VkWriteDescriptorSet> write_decriptor_sets{
        swapchain_image_views.size()};
    for (auto i = 0; i < descriptor_set_layouts.size(); ++i) {
        descriptor_image_info[i] = {.imageView = swapchain_image_views[i],
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
    vkUpdateDescriptorSets(device.device, swapchain_image_views.size(),
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
    VkCommandPoolCreateInfo command_pool_create_info = {};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex =
        device.get_queue_index(vkb::QueueType::present).value();
    VkCommandPool command_pool;
    if (vkCreateCommandPool(device.device, &command_pool_create_info, nullptr,
                            &command_pool) != VK_SUCCESS) {
        std::cerr << "failed to create command pool\n";
        return 1;
    }
    SDL_Event event;
    int quit = 0;
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

            default:
                break;
            }
        }
    }
    return 0;
}
