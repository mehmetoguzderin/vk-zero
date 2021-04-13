#ifndef VK_ZERO_LIB_H
#define VK_ZERO_LIB_H

#ifdef VK_ZERO_CPU

#define VK_NO_PROTOTYPES
#include "volk.h"

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_VULKAN_VERSION 1001000
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "VkBootstrap.h"

#include "SDL.h"
#include "SDL_vulkan.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_vulkan.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include "glm/glm.hpp"
#include "glm/gtx/compatibility.hpp"
using namespace glm;

#define global
#define kernel

uint32_t GLOBAL_ID[3]{0, 0, 0};

inline uint32_t get_global_id(uint32_t dimindx) { return GLOBAL_ID[dimindx]; }

#else

#define int32_t int
#define uint32_t uint
#define int64_t long
#define uint64_t ulong

#define uvec2 (uint2)
#define ivec2 (int2)
#define vec4 (float4)
#define uvec4 (uint4)

#endif

#endif
