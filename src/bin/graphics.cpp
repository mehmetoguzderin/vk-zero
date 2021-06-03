#define VK_ZERO_IMPLEMENTATION
#include "compute.h"

int main(int argc, char *argv[]) {
    try {
        std::vector<const char *> enabledLayers{};
        std::vector<const char *> enabledExtensions{};
        auto window = createWindow(enabledExtensions, 512, 512);
        const auto instance = createInstance(enabledLayers, enabledExtensions);
        const auto surface = createSurface(window, instance);
        const auto device = createDevice(
            instance,
            VkZeroPhysicalDeviceFeatures{
                .features =
                    vk::PhysicalDeviceFeatures{
                        .shaderStorageImageWriteWithoutFormat = true,
                        .shaderInt64 = true,
                    },
                .features11 =
                    vk::PhysicalDeviceVulkan11Features{
                        .variablePointersStorageBuffer = true,
                        .variablePointers = true,
                    },
            },
            std::vector<vk::DescriptorPoolSize>{});
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
        device.destroy();
        instance.destroy();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
    } catch (const int error) {
        std::cout << "Error\n";
    }
    return 0;
}