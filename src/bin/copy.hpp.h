#ifndef COPY_HPP
#define COPY_HPP

#if 0  // clspv -O=3 --spv-version=1.3 --cl-std=CLC++ --cl-single-precision-constant --inline-entry-points --uniform-workgroup-size -o copy.spv copy.hpp

#include "main.h"

template <typename T> struct copy {
    static T *A;
    static T *B;
    static T *C;
};

#ifndef VK_ZERO_CPU

kernel void copy_kernel(const int N) {
    int i = (int)get_global_id(0);
    if (i >= N)
        return;
    copy<float>::A[i] = copy<float>::B[i] + copy<float>::C[i];
}

#endif

#endif

#endif
