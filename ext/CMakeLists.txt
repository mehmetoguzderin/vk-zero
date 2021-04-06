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
