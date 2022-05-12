cmake_minimum_required(VERSION 3.3)

set(CMAKE_C_STANDARD 11)

project(plutovg C)

add_library(plutovg STATIC)
target_compile_options(plutovg PUBLIC -fsanitize=fuzzer-no-link)
target_link_options(plutovg PUBLIC -fsanitize=fuzzer-no-link)

add_subdirectory(include)
add_subdirectory(source)
add_subdirectory(3rdparty/stb)
add_subdirectory(3rdparty/software)
add_subdirectory(fuzz)
add_subdirectory(example)
