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
            std::vector<vk::DescriptorPoolSize>{
                vk::DescriptorPoolSize{
                    .type = vk::DescriptorType::eStorageBuffer,
                    .descriptorCount = 1 << 16,
                },
                vk::DescriptorPoolSize{
                    .type = vk::DescriptorType::eUniformBuffer,
                    .descriptorCount = 1 << 16,
                },
            });
        const auto [elementsBuffer, elements] =
            createBufferWithType<compute::Elements>(
                device,
                vk::BufferCreateInfo{
                    .size = sizeof(compute::Elements) * 8192 * 16,
                    .usage = vk::BufferUsageFlagBits::eStorageBuffer,
                    .sharingMode = vk::SharingMode::eExclusive,
                },
                VmaAllocationCreateInfo{
                    .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
                    .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
                });
        const auto [constantsBuffer, constants] =
            createBufferWithType<compute::Constants>(
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
        constants[0].length = uvec2(elements.size(), 0);
        for (auto i = 0; i < constants[0].length.x * compute::ELEMENT_WIDTH +
                                 constants[0].length.y;
             ++i) {
            elements[0].element[i] = vec4(4.f);
        }
        std::vector<std::vector<VkZeroBinding>> descriptors{
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
        const auto layout = createLayout(device, descriptors, {});
        std::vector<vk::DescriptorSet> sets =
            device.device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo{
                .descriptorPool = device.descriptorPool,
                .descriptorSetCount = 1,
                .pSetLayouts = layout.sets.data(),
            });
        writeSets(device, descriptors, sets);
        const auto module = createModule(device, "compute.hpp.spv");
        uint3 specializationData{16, 16, 1};
        std::vector<vk::SpecializationMapEntry> specializationMapEntries{
            vk::SpecializationMapEntry{
                .constantID = 0,
                .offset = sizeof(uint32_t) * 0,
                .size = sizeof(uint32_t),
            },
            vk::SpecializationMapEntry{
                .constantID = 1,
                .offset = sizeof(uint32_t) * 1,
                .size = sizeof(uint32_t),
            },
            vk::SpecializationMapEntry{
                .constantID = 2,
                .offset = sizeof(uint32_t) * 2,
                .size = sizeof(uint32_t),
            },
        };
        vk::SpecializationInfo specializationInfo{
            .dataSize = sizeof(specializationData),
            .pData = &specializationData,
        };
        specializationInfo.setMapEntries(specializationMapEntries);
        std::string moduleStageName{"computeKernel"};
        const auto pipeline =
            device.device
                .createComputePipeline(
                    {},
                    vk::ComputePipelineCreateInfo{
                        .stage =
                            vk::PipelineShaderStageCreateInfo{
                                .stage = vk::ShaderStageFlagBits::eCompute,
                                .module = module,
                                .pName = moduleStageName.data(),
                                .pSpecializationInfo = &specializationInfo,
                            },
                        .layout = layout.pipeline,
                    })
                .value;
        const auto commandBuffers =
            device.device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{
                .commandPool = device.queue.commandPool,
                .commandBufferCount = 1,
            });
        commandBuffers[0].begin(vk::CommandBufferBeginInfo{
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
        });
        commandBuffers[0].bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                             layout.pipeline, 0, sets, {});
        commandBuffers[0].bindPipeline(vk::PipelineBindPoint::eCompute,
                                       pipeline);
        commandBuffers[0].dispatch(
            (constants[0].length.x * compute::ELEMENT_WIDTH +
             constants[0].length.y) /
                    (specializationData.x * specializationData.y) +
                1,
            1, 1);
        commandBuffers[0].end();
        auto t0 = std::chrono::high_resolution_clock::now();
        device.queue.queue.submit(
            vk::SubmitInfo{}.setCommandBuffers(commandBuffers));
        device.queue.queue.waitIdle();
        auto t1 = std::chrono::high_resolution_clock::now();
        constexpr auto LINE_WIDTH = uint64_t{8};
        for (auto i = 0;
             i < std::min(constants[0].length.x * compute::ELEMENT_WIDTH +
                              constants[0].length.y,
                          LINE_WIDTH * 4);
             ++i) {
            std::cout << elements[0].element[i].x << " ";
            std::cout << elements[0].element[i].y << " ";
            std::cout << elements[0].element[i].z << " ";
            std::cout << elements[0].element[i].w;
            std::cout << ((i % LINE_WIDTH) == (LINE_WIDTH - 1) ? "\n" : " , ");
        }
        device.device.freeCommandBuffers(device.queue.commandPool,
                                         commandBuffers);
        device.device.destroyPipeline(pipeline);
        device.device.destroyShaderModule(module);
        layout.destroy(device);
        constantsBuffer.destroy(device);
        elementsBuffer.destroy(device);
        device.destroy();
        instance.destroy();
        std::chrono::duration<double, std::milli> ms0 = t1 - t0;
        std::cout << "Runtime: " << ms0.count() << "ms\n";
    } catch (const int error) {
        std::cout << "Error\n";
    }
    return 0;
}