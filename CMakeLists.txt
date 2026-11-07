cmake_minimum_required(VERSION 4.3)
project(kurumi_emit C)

set(CMAKE_C_STANDARD 11)

add_executable(kurumi_emit)
add_subdirectory(src)
target_include_directories(kurumi_emit PRIVATE include)
