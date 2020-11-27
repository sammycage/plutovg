cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_STANDARD 11)

project(plutovg C)

set(PLUTOVG_SRC
    source/plutovg.c
    source/plutovg-paint.c
    source/plutovg-geometry.c
    source/plutovg-blend.c
    source/plutovg-rle.c
    source/plutovg-dash.c
    source/plutovg-font.c

    3rdparty/software/sw_ft_math.c
    3rdparty/software/sw_ft_raster.c
    3rdparty/software/sw_ft_stroker.c
)

add_library(${PROJECT_NAME} ${PLUTOVG_SRC})

add_subdirectory(example)

target_include_directories(${PROJECT_NAME}
PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}/source"
    "${CMAKE_CURRENT_LIST_DIR}/3rdparty/software"
    "${CMAKE_CURRENT_LIST_DIR}/3rdparty/stb"
)
