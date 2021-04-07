#ifndef VK_ZERO_LIB_H
#define VK_ZERO_LIB_H

#ifdef VK_ZERO_CPU

#define VK_NO_PROTOTYPES
#include "SDL.h"
#include "SDL_vulkan.h"

#include "VkBootstrap.h"

#include "glm/glm.hpp"
#include "glm/gtx/compatibility.hpp"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_vulkan.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include "volk.h"

#define global
#define kernel

uint32_t global_id[3]{0, 0, 0};

inline uint32_t get_global_id(uint32_t dimindx) { return global_id[dimindx]; }

#else

#define int32_t int
#define uint32_t uint
#define int64_t long
#define uint64_t ulong

#endif

kernel void shared_kernel(global int *in, global int *out, int n) {
    for (int i = 0; i < n; ++i) {
        out[i] = in[i];
    }
}

#ifndef VK_ZERO_CPU

kernel void device_kernel(global int *in, global int *out, int n) {
    for (int i = 0; i < n; ++i) {
        out[i] = in[i];
    }
}

#endif

#endif
