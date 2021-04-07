cmake_minimum_required(VERSION 3.19)
project(vk-zero VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL
  GIT_TAG        main
)
FetchContent_MakeAvailable(SDL2)

FetchContent_Declare(
  Vulkan-Headers
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers
  GIT_TAG        master 
)
FetchContent_MakeAvailable(Vulkan-Headers)

FetchContent_Declare(
  vk-bootstrap
  GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
  GIT_TAG        master
)
FetchContent_MakeAvailable(vk-bootstrap)

# FetchContent_Declare(
#   clspv
#   GIT_REPOSITORY https://github.com/google/clspv
#   GIT_TAG        master
# )
# FetchContent_GetProperties(clspv)
# if(NOT clspv_POPULATED)
#   FetchContent_Populate(clspv)
#   execute_process(
#     COMMAND "python" "utils/fetch_sources.py"
#     WORKING_DIRECTORY "${clspv_SOURCE_DIR}"
#     RESULT_VARIABLE clspv_EXECUTE_RESULT
#     OUTPUT_VARIABLE clspv_EXECUTE_OUTPUT
#   )
#   message(STATUS "clspv: ${clspv_EXECUTE_RESULT} ${clspv_EXECUTE_OUTPUT}")
#   add_subdirectory(${clspv_SOURCE_DIR} ${clspv_BINARY_DIR})
# endif()

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG        master
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG        master
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
  tinygltf
  GIT_REPOSITORY https://github.com/syoyo/tinygltf
  GIT_TAG        master 
)
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF)
FetchContent_MakeAvailable(tinygltf)

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

include_directories(${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
include_directories(${tinygltf_SOURCE_DIR})
include_directories("src")
link_libraries(SDL2-static)
link_libraries(SDL2main)
link_libraries(Vulkan-Headers)
link_libraries(vk-bootstrap)
# link_libraries(clspv)
link_libraries(glm)
link_libraries(volk)
set(
  VK_ZERO_SOURCES
  ${imgui_SOURCE_DIR}/imconfig.h
  ${imgui_SOURCE_DIR}/imgui.h
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.h
  ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.h
)
add_definitions(-DVK_ZERO_CPU)

add_executable(main ${VK_ZERO_SOURCES} "src/bin/main.cpp")
