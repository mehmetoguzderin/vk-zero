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
        // Create buffers
        // Write buffers
        std::vector<std::vector<VkZeroBinding>> bindings{{}};
        const auto layout = createLayout(device, bindings, {});
        // Allocate descriptor sets
        // Update descriptor sets
        const auto shaderModule = createShaderModule(device, "compute.hpp.spv");
        // Create pipeline
        // Create command buffer
        // Record command buffer
        // Submit command buffer
        // Read buffers
        // Clean resources
        device.device.destroyShaderModule(shaderModule);
        layout.destroy(device);
        device.destroy();
        instance.destroy();
    } catch (const int error) {
        std::cout << "Error\n";
    }
    return 0;
}