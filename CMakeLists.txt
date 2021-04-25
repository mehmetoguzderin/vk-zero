cmake_minimum_required(VERSION 3.19)
project(vk-zero VERSION 0.1.0)
include("CMakeBuild.cmake")

add_executable(main ${VK_ZERO_SOURCES} "src/bin/main.cpp")
