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

kernel void shared_kernel(global const int *in, global int *out, int n) {
    for (int i = 0; i < n; ++i) {
        out[i] = in[i];
    }
}

#ifndef VK_ZERO_CPU

kernel void device_kernel(write_only image2d_t output) {
    int2 dimensions = get_image_dim(output);
    int x = (int)get_global_id(0);
    int y = (int)get_global_id(1);
    if (x >= dimensions.x || y >= dimensions.y)
        return;
    write_imagef(output, ivec2(x, y),
                 vec4((float)x / (float)dimensions.x,
                      (float)y / (float)dimensions.y, 1.f, 1.f));
}

#endif

#endif
