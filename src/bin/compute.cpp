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
        const auto [elementsBuffer, elements] = createBuffer<compute::Elements>(
            device,
            vk::BufferCreateInfo{
                .size = sizeof(compute::Elements),
                .usage = vk::BufferUsageFlagBits::eStorageBuffer,
                .sharingMode = vk::SharingMode::eExclusive,
            },
            VmaAllocationCreateInfo{
                .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
            });
        const auto [constantsBuffer, constants] =
            createBuffer<compute::Constants>(
                device,
                vk::BufferCreateInfo{
                    .size = sizeof(compute::Constants),
                    .usage = vk::BufferUsageFlagBits::eUniformBuffer,
                    .sharingMode = vk::SharingMode::eExclusive,
                },
                VmaAllocationCreateInfo{
                    .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
                    .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
                });
        constants[0].weights = vec4(2.f);
        constants[0].length = uvec2(1, compute::ELEMENT_WIDTH);
        for (auto i = 0; i < constants[0].length.y; ++i) {
            elements[0].element[i] = vec4(4.f);
        }
        std::vector<std::vector<VkZeroBinding>> bindings{
            std::vector<VkZeroBinding>{
                VkZeroBinding{
                    .binding =
                        vk::WriteDescriptorSet{
                            .dstBinding = 0,
                            .dstArrayElement = 0,
                            .descriptorCount = 1,
                            .descriptorType =
                                vk::DescriptorType::eStorageBuffer,
                            .pBufferInfo = &elementsBuffer.descriptor,
                        },
                    .stageFlags = vk::ShaderStageFlagBits::eCompute,
                },
                VkZeroBinding{
                    .binding =
                        vk::WriteDescriptorSet{
                            .dstBinding = 1,
                            .dstArrayElement = 0,
                            .descriptorCount = 1,
                            .descriptorType =
                                vk::DescriptorType::eUniformBuffer,
                            .pBufferInfo = &constantsBuffer.descriptor,
                        },
                    .stageFlags = vk::ShaderStageFlagBits::eCompute,
                },
            },
        };
        const auto layout = createLayout(device, bindings, {});
        // Allocate descriptor sets
        // Write descriptor sets
        const auto shaderModule = createShaderModule(device, "compute.hpp.spv");
        // Create pipeline
        // Create command buffer
        // Record command buffer
        // Submit command buffer
        constexpr auto LINE_WIDTH = 8;
        for (auto i = 0; i < constants[0].length.y; ++i) {
            std::cout << elements[0].element[i].x << " ";
            std::cout << elements[0].element[i].y << " ";
            std::cout << elements[0].element[i].z << " ";
            std::cout << elements[0].element[i].w;
            std::cout << ((i % LINE_WIDTH) == (LINE_WIDTH - 1) ? "\n" : " , ");
        }
        // Clean resources
        device.device.destroyShaderModule(shaderModule);
        layout.destroy(device);
        constantsBuffer.destroy(device);
        elementsBuffer.destroy(device);
        device.destroy();
        instance.destroy();
    } catch (const int error) {
        std::cout << "Error\n";
    }
    return 0;
}