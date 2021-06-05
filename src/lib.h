#ifndef VK_ZERO_LIB_H
#define VK_ZERO_LIB_H

#ifdef VK_ZERO_CPU

#include <iostream>
#include <span>
#include <vector>

#include "vulkan/vulkan.hpp"
#ifdef VK_ZERO_IMPLEMENTATION
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

#ifdef VK_ZERO_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#endif
#include "vk_mem_alloc.h"

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
#if !defined(NDEBUG)
    vk::DebugUtilsMessengerEXT debugUtilsMessenger;
#endif

    inline void destroy() const {
#if !defined(NDEBUG)
        instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
#endif
        instance.destroy();
    }
};

struct VkZeroPhysicalDeviceFeatures {
    vk::PhysicalDeviceFeatures features;
    vk::PhysicalDeviceVulkan11Features features11;
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

    inline void destroy() const {
        device.destroyCommandPool(queue.commandPool);
        device.destroyDescriptorPool(descriptorPool);
        vmaDestroyAllocator(allocator);
        device.destroy();
    }
};

struct VkZeroBuffer {
    vk::Buffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
    vk::DescriptorBufferInfo descriptor;

    inline void destroy(const VkZeroDevice &device) const {
        vmaDestroyBuffer(device.allocator, buffer, allocation);
    }
};

struct VkZeroBinding {
    vk::WriteDescriptorSet binding;
    vk::ShaderStageFlags stageFlags;
};

struct VkZeroLayout {
    std::vector<vk::DescriptorSetLayout> sets;
    vk::PipelineLayout pipeline;

    inline void destroy(const VkZeroDevice &device) const {
        device.device.destroyPipelineLayout(pipeline);
        for (const auto &set : sets) {
            device.device.destroyDescriptorSetLayout(set);
        }
    }
};

inline VkZeroInstance
createInstance(std::vector<const char *> &enabledLayers,
               std::vector<const char *> &enabledExtensions) {
    static vk::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
        dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
    vk::ApplicationInfo applicationInfo{
        .apiVersion = VK_API_VERSION_1_1,
    };
#if !defined(NDEBUG)
    enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
    enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    auto instance = vk::createInstance(vk::InstanceCreateInfo{
        .pApplicationInfo = &applicationInfo,
        .enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
        .ppEnabledLayerNames = enabledLayers.data(),
        .enabledExtensionCount =
            static_cast<uint32_t>(enabledExtensions.size()),
        .ppEnabledExtensionNames = enabledExtensions.data(),
    });
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
#if !defined(NDEBUG)
    static PFN_vkDebugUtilsMessengerCallbackEXT debugUtilsMessengerCallback =
        [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
           VkDebugUtilsMessageTypeFlagsEXT messageTypes,
           VkDebugUtilsMessengerCallbackDataEXT const *pCallbackData,
           void *pUserData) -> VkBool32 {
#if !defined(NDEBUG)
        if (pCallbackData->messageIdNumber == 648835635) {
            // UNASSIGNED-khronos-Validation-debug-build-warning-message
            return VK_FALSE;
        }
        if (pCallbackData->messageIdNumber == 767975156) {
            // UNASSIGNED-BestPractices-vkCreateInstance-specialuse-extension
            return VK_FALSE;
        }
#endif

        std::cout << vk::to_string(
                         static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(
                             messageSeverity))
                  << ": "
                  << vk::to_string(
                         static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(
                             messageTypes))
                  << ":\n";
        std::cout << " "
                  << "messageIDName   = <" << pCallbackData->pMessageIdName
                  << ">\n";
        std::cout << " "
                  << "messageIdNumber = " << pCallbackData->messageIdNumber
                  << "\n";
        std::cout << " "
                  << "message         = <" << pCallbackData->pMessage << ">\n";
        if (0 < pCallbackData->queueLabelCount) {
            std::cout << " "
                      << "Queue Labels:\n";
            for (uint8_t i = 0; i < pCallbackData->queueLabelCount; i++) {
                std::cout << "  "
                          << "labelName = <"
                          << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
            }
        }
        if (0 < pCallbackData->cmdBufLabelCount) {
            std::cout << " "
                      << "CommandBuffer Labels:\n";
            for (uint8_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
                std::cout << "  "
                          << "labelName = <"
                          << pCallbackData->pCmdBufLabels[i].pLabelName
                          << ">\n";
            }
        }
        if (0 < pCallbackData->objectCount) {
            std::cout << " "
                      << "Objects:\n";
            for (uint8_t i = 0; i < pCallbackData->objectCount; i++) {
                std::cout << "  "
                          << "Object " << i << "\n";
                std::cout << "   "
                          << "objectType   = "
                          << vk::to_string(static_cast<vk::ObjectType>(
                                 pCallbackData->pObjects[i].objectType))
                          << "\n";
                std::cout << "   "
                          << "objectHandle = "
                          << pCallbackData->pObjects[i].objectHandle << "\n";
                if (pCallbackData->pObjects[i].pObjectName) {
                    std::cout << "   "
                              << "objectName   = <"
                              << pCallbackData->pObjects[i].pObjectName
                              << ">\n";
                }
            }
        }
        return VK_FALSE;
    };
#endif

    return VkZeroInstance {
        .instance = instance,
        .physicalDevices = instance.enumeratePhysicalDevices(),
#if !defined(NDEBUG)
        .debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(
            vk::DebugUtilsMessengerCreateInfoEXT{
                .messageSeverity =
                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                    vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                .messageType =
                    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
                .pfnUserCallback = debugUtilsMessengerCallback,
            }),
#endif
    };
}

inline VkZeroDevice
createDevice(const VkZeroInstance &instance,
             const VkZeroPhysicalDeviceFeatures enabledFeatures,
             const std::vector<vk::DescriptorPoolSize> poolSizes) {
    const auto physicalDeviceIndex = 0;
    if (instance.physicalDevices.size() < physicalDeviceIndex + 1) {
        throw -1;
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
            .pNext = &enabledFeatures.features11,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCreateInfo,
            .pEnabledFeatures = &enabledFeatures.features,
        });
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
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
#define VMA_FETCH_HPP(memberName)                                              \
    .memberName = VULKAN_HPP_DEFAULT_DISPATCHER.memberName
    VmaVulkanFunctions functions{
        VMA_FETCH_HPP(vkGetPhysicalDeviceProperties),
        VMA_FETCH_HPP(vkGetPhysicalDeviceMemoryProperties),
        VMA_FETCH_HPP(vkAllocateMemory),
        VMA_FETCH_HPP(vkFreeMemory),
        VMA_FETCH_HPP(vkMapMemory),
        VMA_FETCH_HPP(vkUnmapMemory),
        VMA_FETCH_HPP(vkFlushMappedMemoryRanges),
        VMA_FETCH_HPP(vkInvalidateMappedMemoryRanges),
        VMA_FETCH_HPP(vkBindBufferMemory),
        VMA_FETCH_HPP(vkBindImageMemory),
        VMA_FETCH_HPP(vkGetBufferMemoryRequirements),
        VMA_FETCH_HPP(vkGetImageMemoryRequirements),
        VMA_FETCH_HPP(vkCreateBuffer),
        VMA_FETCH_HPP(vkDestroyBuffer),
        VMA_FETCH_HPP(vkCreateImage),
        VMA_FETCH_HPP(vkDestroyImage),
        VMA_FETCH_HPP(vkCmdCopyBuffer),
        VMA_FETCH_HPP(vkGetBufferMemoryRequirements2KHR),
        VMA_FETCH_HPP(vkGetImageMemoryRequirements2KHR),
        VMA_FETCH_HPP(vkBindBufferMemory2KHR),
        VMA_FETCH_HPP(vkBindImageMemory2KHR),
        VMA_FETCH_HPP(vkGetPhysicalDeviceMemoryProperties2KHR),
    };
#undef VMA_FETCH_HPP
    VmaAllocatorCreateInfo allocatorCreateInfo = {
        .physicalDevice = instance.physicalDevices[physicalDeviceIndex],
        .device = device,
        .pVulkanFunctions = &functions,
        .instance = instance.instance,
        .vulkanApiVersion = VK_API_VERSION_1_0,
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
                .maxSets = 1 << 16,
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

inline VkZeroBuffer
createBuffer(const VkZeroDevice &device,
             const vk::BufferCreateInfo bufferCreateInfo,
             const VmaAllocationCreateInfo allocationCreateInfo) {
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
    VkBufferCreateInfo vkBufferCreateInfo = bufferCreateInfo;
    if (vmaCreateBuffer(device.allocator, &vkBufferCreateInfo,
                        &allocationCreateInfo, &buffer, &allocation,
                        &info) != VK_SUCCESS) {
        throw -1;
    }
    return VkZeroBuffer{
        .buffer = buffer,
        .allocation = allocation,
        .info = info,
        .descriptor =
            vk::DescriptorBufferInfo{
                .buffer = buffer,
                .offset = 0,
                .range = bufferCreateInfo.size,
            },
    };
}

template <typename T>
std::tuple<VkZeroBuffer, std::span<T>>
createBufferWithType(const VkZeroDevice &device,
                     const vk::BufferCreateInfo bufferCreateInfo,
                     const VmaAllocationCreateInfo allocationCreateInfo) {
    auto buffer = createBuffer(device, bufferCreateInfo, allocationCreateInfo);
    return std::make_tuple(
        buffer, std::span{reinterpret_cast<T *>(buffer.info.pMappedData),
                          bufferCreateInfo.size / sizeof(T)});
}

inline VkZeroLayout
createLayout(const VkZeroDevice &device,
             const std::vector<std::vector<VkZeroBinding>> &descriptors,
             const std::vector<vk::PushConstantRange> pushConstantRanges) {
    std::vector<vk::DescriptorSetLayout> sets{};
    for (const auto &bindings : descriptors) {
        std::vector<vk::DescriptorSetLayoutBinding> setBindings{};
        for (const auto &binding : bindings) {
            setBindings.push_back(vk::DescriptorSetLayoutBinding{
                .binding = binding.binding.dstBinding,
                .descriptorType = binding.binding.descriptorType,
                .descriptorCount = binding.binding.descriptorCount,
                .stageFlags = binding.stageFlags,
            });
            if (binding.binding.pImageInfo) {
                if (binding.binding.pImageInfo->sampler) {
                    setBindings.back().setPImmutableSamplers(
                        &binding.binding.pImageInfo->sampler);
                }
            }
        }
        vk::DescriptorSetLayoutCreateInfo setCreateInfo{};
        setCreateInfo.setBindings(setBindings);
        sets.push_back(device.device.createDescriptorSetLayout(setCreateInfo));
    }
    vk::PipelineLayoutCreateInfo pipelineCreateInfo{};
    pipelineCreateInfo.setSetLayouts(sets);
    pipelineCreateInfo.setPushConstantRanges(pushConstantRanges);
    return VkZeroLayout{
        .sets = sets,
        .pipeline = device.device.createPipelineLayout(pipelineCreateInfo),
    };
}

inline void
writeSets(const VkZeroDevice &device,
          const std::vector<std::vector<VkZeroBinding>> &descriptors,
          const std::vector<vk::DescriptorSet> &sets) {
    for (auto i = 0; const auto &bindings : descriptors) {
        std::vector<vk::WriteDescriptorSet> writes{};
        for (auto j = 0; const auto &binding : bindings) {
            writes.push_back(binding.binding);
            writes.back().dstSet = sets[i];
            ++j;
        }
        device.device.updateDescriptorSets(writes, {});
        ++i;
    }
}

inline vk::ShaderModule createModule(const VkZeroDevice &device,
                                     const std::string source) {
    std::ifstream file(source, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw -1;
    }
    size_t file_size = (size_t)file.tellg();
    std::vector<char> code(file_size);
    file.seekg(0);
    file.read(code.data(), static_cast<std::streamsize>(file_size));
    return device.device.createShaderModule(vk::ShaderModuleCreateInfo{
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t *>(code.data()),
    });
}

inline GLFWwindow *createWindow(std::vector<const char *> &enabledExtensions,
                                const uint32_t width, const uint32_t height) {
    if (glfwInit() != GLFW_TRUE) {
        throw -1;
    }
    if (glfwVulkanSupported() != GLFW_TRUE) {
        throw -1;
    }
    if (!ImGui::CreateContext()) {
        throw -1;
    }
    ImGui::StyleColorsDark();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window = glfwCreateWindow(width, height, "", nullptr, nullptr);
    if (!window) {
        throw -1;
    }
    glfwSetWindowSizeLimits(window, 8, 8, GLFW_DONT_CARE, GLFW_DONT_CARE);
    uint32_t requiredInstanceExtensionsCount = 0;
    const auto requiredInstanceExtensions =
        glfwGetRequiredInstanceExtensions(&requiredInstanceExtensionsCount);
    for (auto i = 0; i < requiredInstanceExtensionsCount; ++i) {
        enabledExtensions.push_back(requiredInstanceExtensions[i]);
    }
    return window;
}

inline VkSurfaceKHR createSurface(GLFWwindow *&window,
                                  const VkZeroInstance &instance) {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance.instance, window, nullptr, &surface) !=
        VK_SUCCESS) {
        throw -1;
    }
    ImGui_ImplGlfw_InitForVulkan(window, false);
    VkInstance vkInstance = instance.instance;
    ImGui_ImplVulkan_LoadFunctions(
        [](const char *functionName, void *instance) {
            return VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr(
                *static_cast<VkInstance *>(instance), functionName);
        },
        static_cast<void *>(&vkInstance));
    return surface;
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

struct Pcg32 {
    uint64_t state;
    uint64_t inc;
};

inline Pcg32 Pcg32Seed(uint64_t state, uint64_t sequence) {
    Pcg32 rng;
    rng.state = 0U;
    rng.inc = (sequence << 1u) | 1u;
    rng.state = rng.state * 6364136223846793005ULL + rng.inc;
    rng.state += state;
    rng.state = rng.state * 6364136223846793005ULL + rng.inc;
    return rng;
}

inline uint32_t Pcg32Integer(Pcg32 &rng) {
    auto oldState = rng.state;
    rng.state = oldState * 6364136223846793005ULL + (rng.inc | 1);
    uint32_t xorShifted = ((oldState >> 18u) ^ oldState) >> 27u;
    uint32_t rot = oldState >> 59u;
    return (xorShifted >> rot) | (xorShifted << ((-rot) & 31));
}

inline float Pcg32Float(Pcg32 &rng) {
    union {
        uint32_t integer_type;
        float float_type;
    } value;
    value.integer_type = (Pcg32Integer(rng) >> 9) | 0x3f800000u;
    return value.float_type - 1.0f;
}

#endif
