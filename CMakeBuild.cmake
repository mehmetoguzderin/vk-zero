set(CMAKE_CXX_STANDARD 20)

if(APPLE)
  set(CMAKE_CXX_FLAGS "-Wno-nullability-completeness")
endif()

add_compile_definitions(VK_ZERO_CPU)
add_compile_definitions(VK_NO_PROTOTYPES)
add_compile_definitions(VMA_STATIC_VULKAN_FUNCTIONS=0)
add_compile_definitions(VMA_DYNAMIC_VULKAN_FUNCTIONS=1)
add_compile_definitions(VMA_VULKAN_VERSION=1001000)

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
add_definitions(-DIMGUI_IMPL_VULKAN_NO_PROTOTYPES)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
  tinygltf
  GIT_REPOSITORY https://github.com/syoyo/tinygltf
  GIT_TAG        master
)
FetchContent_GetProperties(tinygltf)
if(NOT tinygltf_POPULATED)
  set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF)
  set(tinygltf_TINYGLTF_BUILD_LOADER_EXAMPLE OFF)
  add_definitions(-DTINYGLTF_BUILD_LOADER_EXAMPLE=OFF)
  FetchContent_Populate(tinygltf)
  add_subdirectory(${tinygltf_SOURCE_DIR} ${tinygltf_BINARY_DIR})
endif()

FetchContent_Declare(
  vma
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
  GIT_TAG        master
)
FetchContent_MakeAvailable(vma)

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
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lib.cpp
)

add_library(vk-zero STATIC ${VK_ZERO_SOURCES})

file(GLOB kernel-header-files "${CMAKE_CURRENT_SOURCE_DIR}/src/bin/*.h")
foreach(kernel-header ${kernel-header-files})
  get_filename_component(kernel-header ${kernel-header} NAME)
  list(APPEND kernel-headers "${CMAKE_CURRENT_SOURCE_DIR}/src/bin/${kernel-header}")
endforeach()
list(APPEND kernel-headers "${CMAKE_CURRENT_SOURCE_DIR}/src/lib.h")
file(GLOB kernels "${CMAKE_CURRENT_SOURCE_DIR}/src/bin/*.hpp")
foreach(kernel ${kernels})
  get_filename_component(kernel ${kernel} NAME)
  if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    find_program(CLSPV_WIN "clspv")
    if(CLSPV_WIN)
      set(
        CLSPV_COMMAND
        clspv -O=3
              --spv-version=1.3
              --cl-std=CLC++
              --cl-single-precision-constant
              --inline-entry-points
              --uniform-workgroup-size
              --constant-args-ubo
              -o "${kernel}"
              "bin/${kernel}"
      )
    else()
      set(
        CLSPV_COMMAND
        PowerShell
        wsl
        clspv -O=3
              --spv-version=1.3
              --cl-std=CLC++
              --cl-single-precision-constant
              --inline-entry-points
              --uniform-workgroup-size
              --constant-args-ubo
              -o "${kernel}"
              "bin/${kernel}"
      )
    endif()
  else()
    set(
      CLSPV_COMMAND
      clspv -O=3
            --spv-version=1.3
            --cl-std=CLC++
            --cl-single-precision-constant
            --inline-entry-points
            --uniform-workgroup-size
            --constant-args-ubo
            -o "${kernel}"
            "bin/${kernel}"
    )
  endif()
  add_custom_command(
    OUTPUT "${CMAKE_BINARY_DIR}/${kernel}"
    COMMAND ${CMAKE_COMMAND} -E rm -f
                             "${CMAKE_BINARY_DIR}/${kernel}"
    COMMAND ${CLSPV_COMMAND}
    COMMAND spirv-opt --strip-reflect
                      -O
                      -o "${kernel}"
                      "${kernel}"
    COMMAND ${CMAKE_COMMAND} -E copy
                             "${kernel}"
                             "${CMAKE_BINARY_DIR}/${kernel}"
    COMMAND ${CMAKE_COMMAND} -E rm -f
                             "${CMAKE_CURRENT_SOURCE_DIR}/src/${kernel}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/bin/${kernel}" ${kernel-headers}
    VERBATIM
    COMMAND_EXPAND_LISTS
  )
  list(APPEND clspv-kernels "${CMAKE_BINARY_DIR}/${kernel}")
endforeach()
add_custom_target(
  clspv-target ALL
  DEPENDS ${clspv-kernels} ${kernel-headers}
)

target_include_directories(vk-zero PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_include_directories(vk-zero PUBLIC ${tinygltf_SOURCE_DIR})
target_include_directories(vk-zero PUBLIC ${vma_SOURCE_DIR}/include)
target_link_libraries(vk-zero PUBLIC SDL2-static)
target_link_libraries(vk-zero PUBLIC SDL2main)
target_link_libraries(vk-zero PUBLIC Vulkan-Headers)
target_link_libraries(vk-zero PUBLIC vk-bootstrap)
# link_libraries(clspv)
target_link_libraries(vk-zero PUBLIC glm)
target_link_libraries(vk-zero PUBLIC volk)

link_libraries(vk-zero)
add_dependencies(vk-zero clspv-target)
