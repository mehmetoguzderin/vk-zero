#ifndef COMPUTE_H
#define COMPUTE_H

#include "../lib.h"

#ifdef VK_ZERO_CPU

#include <iostream>

#else

#endif

constexpr uint64_t ELEMENT_WIDTH = 2048;
struct ComputeElements {
    float4 element[ELEMENT_WIDTH];
};
constexpr uint64_t ELEMENT_SIZE = sizeof(float4);

struct ComputeConstants {
    float4 weights;
    uint2 length;
};

#ifdef VK_ZERO_CPU

#else

#endif

#endif