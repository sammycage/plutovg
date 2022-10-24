cmake_minimum_required(VERSION 3.3)

set(CMAKE_C_STANDARD 11)

project(plutovg C)

add_library(plutovg STATIC)

add_subdirectory(include)
add_subdirectory(source)
add_subdirectory(example)
