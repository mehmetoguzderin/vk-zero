#ifndef VK_ZERO_LIB_H
#define VK_ZERO_LIB_H

#ifdef VK_ZERO_CPU
#define VK_ZERO_IMPLEMENTATION

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

inline static uint32_t GLOBAL_ID[3]{0, 0, 0};

inline uint32_t get_global_id(uint32_t dimindx) { return GLOBAL_ID[dimindx]; }

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
