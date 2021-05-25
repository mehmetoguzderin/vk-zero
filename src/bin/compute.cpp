#define VK_ZERO_IMPLEMENTATION
#include "compute.h"

int main(int argc, char *argv[]) {
    try {
        std::vector<const char *> enabledExtensions{};
        const auto instance = createInstance(enabledExtensions);
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
        device.destroy();
        instance.destroy();
    } catch (const int error) {
        std::cout << "Error\n";
    }
    return 0;
}