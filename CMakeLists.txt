cmake_minimum_required(VERSION 3.19)
project(vk-zero VERSION 0.1.0)
include("CMakeBuild.cmake")

add_executable(main "src/bin/main.cpp")
add_executable(compute_weighted_add "src/bin/compute_weighted_add.cpp")
