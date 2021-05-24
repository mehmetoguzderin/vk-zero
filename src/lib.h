#ifndef VK_ZERO_LIB_H
#define VK_ZERO_LIB_H

#ifdef VK_ZERO_CPU

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "vulkan/vulkan.hpp"
#ifdef VK_ZERO_IMPLEMENTATION
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

#include "volk.h"

#ifdef VK_ZERO_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#endif
#include "vk_mem_alloc.h"

#include "VkBootstrap.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#ifdef VK_ZERO_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include "tiny_gltf.h"

#include "glm/glm.hpp"
#include "glm/gtx/compatibility.hpp"
using namespace glm;

#define uint1 uvec1
#define uint2 uvec2
#define uint3 uvec3
#define uint4 uvec4

#define __kernel
#define __global
#define __constant const

#ifdef VK_ZERO_IMPLEMENTATION
static uint32_t GLOBAL_ID[3]{0, 0, 0};
#endif

inline uint32_t get_global_id(uint32_t dimindx) { return GLOBAL_ID[dimindx]; }

struct VkZeroInstance {
    vk::Instance instance;
    std::vector<vk::PhysicalDevice> physicalDevices;
    std::vector<vk::PhysicalDeviceGroupProperties> physicalDeviceGroups;
};

struct VkZeroQueue {
    uint32_t familyIndex;
    uint32_t index;
    float priority;
    vk::Queue queue;
    vk::CommandPool commandPool;
};

struct VkZeroDevice {
    uint32_t index;
    vk::Device device;
    VmaAllocator allocator;
    vk::DescriptorPool descriptorPool;
    VkZeroQueue queue;
};

inline VkZeroInstance
createInstance(const uint32_t enabledExtensionCount,
               const char *const *ppEnabledExtensionNames) {
    vk::ApplicationInfo applicationInfo{
        .apiVersion = VK_API_VERSION_1_1,
    };
    auto instance = vk::createInstance(vk::InstanceCreateInfo{
        .pApplicationInfo = &applicationInfo,
        .enabledExtensionCount = enabledExtensionCount,
        .ppEnabledExtensionNames = ppEnabledExtensionNames,
    });
    return VkZeroInstance{
        .instance = instance,
        .physicalDevices = instance.enumeratePhysicalDevices(),
        .physicalDeviceGroups = instance.enumeratePhysicalDeviceGroups(),
    };
}

inline VkZeroDevice
createDevice(const VkZeroInstance &instance,
             const vk::PhysicalDeviceFeatures *pEnabledFeatures,
             std::vector<vk::DescriptorPoolSize> poolSizes) {
    const auto physicalDeviceIndex = 0;
    if (instance.physicalDevices.size() < physicalDeviceIndex + 1) {
        throw static_cast<int32_t>(-1);
    }
    const auto queueFamilyIndex = 0;
    const auto queueIndex = 0;
    const auto queuePriority = 0.f;
    vk::DeviceQueueCreateInfo queueCreateInfo{
        .queueFamilyIndex = queueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority};
    auto device = instance.physicalDevices[physicalDeviceIndex].createDevice(
        vk::DeviceCreateInfo{
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCreateInfo,
            .pEnabledFeatures = pEnabledFeatures,
        });
    if (instance.physicalDevices[physicalDeviceIndex]
            .getQueueFamilyProperties()
            .size() < queueFamilyIndex + 1) {
        throw -1;
    }
    const auto queueFamilyProperties =
        instance.physicalDevices[physicalDeviceIndex]
            .getQueueFamilyProperties()[queueFamilyIndex];
    const auto queueFlags = vk::QueueFlagBits::eGraphics |
                            vk::QueueFlagBits::eCompute |
                            vk::QueueFlagBits::eTransfer;
    if ((queueFamilyProperties.queueFlags & queueFlags) != queueFlags &&
        queueFamilyProperties.queueCount < queueIndex + 1) {
        throw -1;
    }
    VmaAllocatorCreateInfo allocatorCreateInfo = {
        .physicalDevice = instance.physicalDevices[physicalDeviceIndex],
        .device = device,
        .instance = instance.instance,
        .vulkanApiVersion = VK_API_VERSION_1_1,
    };
    VmaAllocator allocator;
    if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS) {
        throw -1;
    }
    return VkZeroDevice{
        .index = physicalDeviceIndex,
        .device = device,
        .allocator = allocator,
        .descriptorPool =
            device.createDescriptorPool(vk::DescriptorPoolCreateInfo{
                .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                .maxSets = 8192,
                .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                .pPoolSizes = poolSizes.data(),
            }),
        .queue =
            VkZeroQueue{
                .familyIndex = queueFamilyIndex,
                .index = queueIndex,
                .priority = queuePriority,
                .queue = device.getQueue(queueFamilyIndex, queueIndex),
                .commandPool =
                    device.createCommandPool(vk::CommandPoolCreateInfo{
                        .flags =
                            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                        .queueFamilyIndex = queueFamilyIndex,
                    }),
            },
    };
}

#else

#define int32_t int
#define uint32_t uint
#define int64_t long
#define uint64_t ulong

#define uvec1 (uint1)
#define uvec2 (uint2)
#define uvec3 (uint3)
#define uvec4 (uint4)

#define ivec1 (int1)
#define ivec2 (int2)
#define ivec3 (int3)
#define ivec4 (int4)

#define vec1 (float1)
#define vec2 (float2)
#define vec3 (float3)
#define vec4 (float4)

#endif

typedef struct {
    uint64_t state;
    uint64_t inc;
} pcg32_random_t;

uint32_t pcg32_random_r(pcg32_random_t *rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

#endif
