#ifndef COMPUTE_HPP
#define COMPUTE_HPP

#include "compute.h"

#ifdef VK_ZERO_CPU

#include <iostream>

#else

#endif

template <typename A> A compute_function(const A i, const A weights) {
    return i * weights;
}

#ifdef VK_ZERO_CPU

#else

__kernel void compute_kernel(__global ComputeElements *elements,
                             __constant ComputeConstants *constants) {
    uint64_t length = static_cast<uint64_t>(constants->length.x) *
                          static_cast<uint64_t>(ELEMENT_WIDTH) +
                      static_cast<uint64_t>(constants->length.y);
    uint64_t i = static_cast<uint64_t>(get_global_id(0)) *
                     static_cast<uint64_t>(get_local_size(1)) +
                 static_cast<uint64_t>(get_local_id(1));
    if (i >= length)
        return;
    uint64_t x = i / ELEMENT_WIDTH;
    uint64_t y = i % ELEMENT_WIDTH;
    elements[x].element[y] =
        compute_function(elements[x].element[y], constants->weights);
}

#endif

#endif
