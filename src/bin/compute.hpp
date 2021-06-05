#ifndef COMPUTE_HPP
#define COMPUTE_HPP

#include "compute.h"

#ifdef VK_ZERO_CPU

#include <iostream>

#else

#endif

template <typename A> A computeFunction(const A i, const A weights) {
    return i * weights;
}

#ifdef VK_ZERO_CPU

#else

__kernel void computeKernel(__global compute::Elements *elements,
                            __constant compute::Constants *constants) {
    uint64_t size = constants->size;
    uint64_t i = static_cast<uint64_t>(get_global_id(0)) *
                     static_cast<uint64_t>(get_local_size(1)) +
                 static_cast<uint64_t>(get_local_id(1));
    if (i >= size)
        return;
    elements[i].element =
        computeFunction(elements[i].element, constants->weights);
}

#endif

#endif
