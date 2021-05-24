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

struct VkZeroDevice {
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    vk::PhysicalDeviceProperties properties;
    vk::PhysicalDeviceFeatures features;
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
};

struct VkZeroQueue {
    vk::Queue queue;
    uint32_t familyIndex;
    uint32_t index;
};

VkZeroQueue getGraphicsComputeTransferQueue(const VkZeroDevice &device) {
    if (device.queueFamilyProperties.size() < 1) {
        throw static_cast<int32_t>(-1);
    }
    const auto &familyProperties = device.queueFamilyProperties[0];
    const auto queueFlags = vk::QueueFlagBits::eGraphics |
                            vk::QueueFlagBits::eCompute |
                            vk::QueueFlagBits::eTransfer;
    if ((familyProperties.queueFlags & queueFlags) != queueFlags) {
        throw static_cast<int32_t>(-1);
    }
    return VkZeroQueue{
        .queue = device.device.getQueue(0, 0), .familyIndex = 0, .index = 0};
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

#endif
