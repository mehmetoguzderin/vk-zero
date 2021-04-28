#ifndef COMPUTE_WEIGHTED_ADD_HPP
#define COMPUTE_WEIGHTED_ADD_HPP

#include "compute_weighted_add.h"

template <typename T>
T weighted_add(float4 weights, const T &b, const T &c, const T &d) {
    return weights.x * (b * weights.y + c * weights.z + d * weights.w);
}

#ifndef VK_ZERO_CPU

__kernel void
compute_weighted_add_kernel(__global ComputeWeightedAddElement *a,
                            __global ComputeWeightedAddElement *b,
                            __global ComputeWeightedAddElement *c,
                            __global ComputeWeightedAddElement *d,
                            __constant ComputeWeightedAddConstants *constants) {
    uint64_t length = (uint64_t)constants->length.x * (uint64_t)ELEMENT_WIDTH +
                      (uint64_t)constants->length.y;
    uint64_t i = (uint64_t)get_global_id(0) * (uint64_t)get_local_size(1) +
                 (uint64_t)get_local_id(1);
    if (i >= length)
        return;
    uint64_t x = i / ELEMENT_WIDTH;
    uint64_t y = i % ELEMENT_WIDTH;
    a[x].element[y] = weighted_add(constants->weights, b[x].element[y],
                                   c[x].element[y], d[x].element[y]);
}

#endif

#endif
