# `vk-zero`

## Prerequisites

- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
- [Clspv](https://github.com/google/clspv)
  - On Windows, having `clspv` inside WSL is also acceptable

## Build

```sh
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
