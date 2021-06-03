#ifndef COMPUTE_H
#define COMPUTE_H

#include "../lib.h"

#ifdef VK_ZERO_CPU

#else

#endif

namespace compute {
using Element = float4;
constexpr uint64_t ELEMENT_SIZE = sizeof(Element);
constexpr uint64_t ELEMENT_WIDTH = 2048;
struct Elements {
    Element element[ELEMENT_WIDTH];
};

struct Constants {
    float4 weights;
    uint2 length;
};
} // namespace compute

#ifdef VK_ZERO_CPU

#else

#endif

#endif