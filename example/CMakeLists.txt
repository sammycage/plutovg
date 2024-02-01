cmake_minimum_required(VERSION 3.3)

set(CMAKE_C_STANDARD 99)

project(examples C)

add_executable(smiley smiley.c)
target_link_libraries(smiley plutovg m)

add_executable(texture texture.c)
target_link_libraries(texture plutovg m)

file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
