cmake_minimum_required(VERSION 3.19)
project(vk-zero VERSION 0.1.0)

include(FetchContent)
FetchContent_Declare(
    fetch_vulkan_headers
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers
    GIT_TAG        master 
)
FetchContent_MakeAvailable(fetch_vulkan_headers)
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
endif()
FetchContent_Declare(
    fetch_volk
    GIT_REPOSITORY https://github.com/zeux/volk
    GIT_TAG        master
)
FetchContent_MakeAvailable(fetch_volk)
FetchContent_Declare(
    fetch_vk_bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG        master
)
FetchContent_MakeAvailable(fetch_vk_bootstrap)
FetchContent_Declare(
    fetch_sdl2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    GIT_TAG        main
)
FetchContent_MakeAvailable(fetch_sdl2)

add_executable(main "src/main.cpp")
set_property(TARGET main PROPERTY CXX_STANDARD 20)
target_link_libraries(main Vulkan-Headers)
target_link_libraries(main volk)
target_link_libraries(main vk-bootstrap)
target_link_libraries(main SDL2)
