# `vk-zero`

## Usage

### Rules

1. Do not create any file with `hpp` extension inside `src` directory except inside `src/bin` subdirectory, you can create files with `hpp` extension inside `src/bin` subdirectory.

## Tooling

### Prerequisites

- [CMake](https://cmake.org)
  - On Windows, recent Visual Studio versions [can open CMake projects directly](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio), which you can use instead.
- [Clang](https://clang.llvm.org)
  - On Windows, recent Visual Studio versions [can install Clang](https://docs.microsoft.com/en-us/cpp/build/clang-support-msbuild), which you can use instead.
- [Clspv](https://github.com/google/clspv)
  - On Windows, having `clspv` inside [WSL2](https://docs.microsoft.com/en-us/windows/wsl) is also acceptable.
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)

### Building (CLI)

#### Debug Profile

```sh
cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

#### Release Profile

```sh
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
