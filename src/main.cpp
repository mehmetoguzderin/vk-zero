#define VK_NO_PROTOTYPES
#include "VkBootstrap.h"
#include "volk.h"
#include <iostream>

int main() {
    if (auto result = volkInitialize(); result != VK_SUCCESS) {
        std::cerr << "Failed to initialize Volk. Error: " << result << "\n";
    }
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("main")
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
    return 0;
}
