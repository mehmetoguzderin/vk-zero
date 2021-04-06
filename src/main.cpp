#define VK_NO_PROTOTYPES
#include "SDL.h"
#include "SDL_vulkan.h"
#include "VkBootstrap.h"
#include "volk.h"
#include <iostream>

int main() {
    auto app_name = "main";
    if (auto result = volkInitialize(); result != VK_SUCCESS) {
        std::cerr << "Failed to initialize Volk. Error: " << result << "\n";
        return 1;
    }
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name(app_name)
                        .request_validation_layers()
                        .use_default_debug_messenger()
                        .build();
    if (!inst_ret) {
        std::cerr << "Failed to create Vulkan instance. Error: "
                  << inst_ret.error().message() << "\n";
        return 1;
    }
    vkb::Instance vkb_inst = inst_ret.value();
    volkLoadInstance(vkb_inst.instance);
    if (auto result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS); result < 0) {
        std::cerr << "Failed to initialize SDL. Error: " << SDL_GetError()
                  << "\n";
        return 1;
    }
    SDL_CreateWindow(app_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                     512, 512, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return 0;
            }
        }
    }
    return 0;
}
