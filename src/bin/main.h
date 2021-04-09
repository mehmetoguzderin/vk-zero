#include "lib.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

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

std::optional<int> create_window_instance_surface_device(
    const char *&name, SDL_Window *&window, vkb::Instance &instance,
    VkSurfaceKHR &surface, vkb::Device &device) {
    window = SDL_CreateWindow(
        name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) {
        return -1;
    }
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
    vkb::PhysicalDevice physical_device;
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

std::optional<int> create_swapchain(vkb::Device &device,
                                    vkb::Swapchain &swapchain,
                                    std::vector<VkImage> &images,
                                    std::vector<VkImageView> &image_views,
                                    bool destroy = false) {
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
    return {};
}

std::optional<int>
create_set_layout(vkb::Device &device,
                  VkDescriptorSetLayout &descriptor_set_layout) {
    VkDescriptorSetLayoutBinding binding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .pImmutableSamplers = VK_NULL_HANDLE};
    VkDescriptorSetLayoutCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .bindingCount = 1,
        .pBindings = &binding};
    if (vkCreateDescriptorSetLayout(device.device, &create_info, VK_NULL_HANDLE,
                                    &descriptor_set_layout) != VK_SUCCESS) {
        return -1;
    }
    return {};
}