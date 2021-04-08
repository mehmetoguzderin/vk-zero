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
    VkQueue queue;
    if (auto result = device.get_queue(vkb::QueueType::present);
        !result.has_value()) {
        std::cerr << "failed to get queue: " << result.error().message()
                  << "\n";
        return 1;
    } else {
        queue = result.value();
    }
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
