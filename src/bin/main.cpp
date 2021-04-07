
#include "lib.h"

#include <iostream>

int main(int argc, char *argv[]) {
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
