cmake_minimum_required(VERSION 3.19)

add_executable(main "main.cpp")
set_property(TARGET main PROPERTY CXX_STANDARD 20)

target_link_libraries(main Vulkan-Headers)
target_link_libraries(main volk)
target_link_libraries(main vk-bootstrap)
target_link_libraries(main SDL2)
