#ifndef COMPUTE_H
#define COMPUTE_H

#include "../lib.h"

#ifdef VK_ZERO_CPU

#else

#endif

namespace compute {
using Element = float4;
struct Elements {
    Element element;
};

struct Constants {
    float4 weights;
    uint64_t size;
};
} // namespace compute

#ifdef VK_ZERO_CPU

#else

#endif

#endif