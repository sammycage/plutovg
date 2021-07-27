cmake_minimum_required(VERSION 3.3)

set(CMAKE_C_STANDARD 99)

project(smiley C)

add_executable(smiley smiley.c)
target_link_libraries(smiley plutovg m)
