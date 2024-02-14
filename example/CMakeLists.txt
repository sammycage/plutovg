cmake_minimum_required(VERSION 3.8)

project(smiley C)

include(../cmake/top.cmake)

if(PROJECT_IS_TOP_LEVEL)
  find_package(plutovg REQUIRED)
endif()

add_executable(smiley smiley.c)
target_link_libraries(smiley PRIVATE plutovg::plutovg)
