#ifndef MAIN_HPP
#define MAIN_HPP

#include "main.h"

template <typename T> struct prefix_exclusive_sum_t4 {
    T data[4];

    prefix_exclusive_sum_t4 prefix_exclusive_sum() {
        return {{0, data[0], data[0] + data[1], data[0] + data[1] + data[2]}};
    }
};

#ifndef VK_ZERO_CPU

__kernel void device_kernel(read_write image2d_t output,
                            __constant MainConstants *constants) {
    int2 dimensions = get_image_dim(output);
    int x = (int)get_global_id(0);
    int y = (int)get_global_id(1);
    if (x >= dimensions.x || y >= dimensions.y)
        return;
    float4 pixel = read_imagef(output, ivec2(x, y));
    if (pixel.w > 0.0)
        return;
    write_imagef(
        output, ivec2(x, y),
        pixel + vec4((float)x / (float)dimensions.x,
                     (float)y / (float)dimensions.y,
                     prefix_exclusive_sum_t4<float>{{0.25f, 0.25f, 0.25f, 0.f}}
                         .prefix_exclusive_sum()
                         .data[3],
                     1.f) *
                    constants->color);
}

#endif

#endif
