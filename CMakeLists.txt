cmake_minimum_required(VERSION 3.19)
project(vk-zero VERSION 0.1.0)

include(FetchContent)
FetchContent_Declare(
  Vulkan-Headers
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers
  GIT_TAG        master 
)
FetchContent_MakeAvailable(Vulkan-Headers)
FetchContent_Declare(
  volk
  GIT_REPOSITORY https://github.com/zeux/volk
  GIT_TAG        master
)
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL Linux)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
endif()
FetchContent_MakeAvailable(volk)
FetchContent_Declare(
  vk-bootstrap
  GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
  GIT_TAG        master
)
FetchContent_MakeAvailable(vk-bootstrap)
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL
  GIT_TAG        main
)
FetchContent_MakeAvailable(SDL2)
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG        master
)
FetchContent_MakeAvailable(glm)

add_executable(main "src/main.cpp")
set_property(TARGET main PROPERTY CXX_STANDARD 20)
target_link_libraries(main PRIVATE Vulkan-Headers)
target_link_libraries(main PRIVATE volk)
target_link_libraries(main PRIVATE vk-bootstrap)
target_link_libraries(main PRIVATE SDL2)
target_link_libraries(main PRIVATE glm)
