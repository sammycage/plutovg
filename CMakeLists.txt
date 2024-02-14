cmake_minimum_required(VERSION 3.8)

project(plutovg VERSION 1.0 LANGUAGES C)

include(cmake/prologue.cmake)
include(CheckLibraryExists)

add_library(
    plutovg STATIC
    source/plutovg-blend.c
    source/plutovg-ft-math.c
    source/plutovg-ft-raster.c
    source/plutovg-ft-stroker.c
    source/plutovg-geometry.c
    source/plutovg-paint.c
    source/plutovg-rle.c
    source/plutovg-stb-png.c
    source/plutovg-stroke.c
    source/plutovg.c
)
add_library(plutovg::plutovg ALIAS plutovg)

target_include_directories(
    plutovg ${warning_guard} PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)
target_include_directories(plutovg ${warning_guard} PRIVATE source)

if(WIN32)
  target_compile_definitions(plutovg PRIVATE _CRT_SECURE_NO_WARNINGS=1)
endif()

check_library_exists(m sqrt "" HAVE_LIBM)
if(HAVE_LIBM)
  target_link_libraries(plutovg PUBLIC m)
endif()

# For compilers that CMake hasn't been taught yet, like MSVC
if("c_std_99" IN_LIST CMAKE_C_COMPILE_FEATURES)
  target_compile_features(plutovg PUBLIC c_std_99)
endif()

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_EXAMPLE "Build example tree." OFF)
  if(BUILD_EXAMPLE)
    add_subdirectory(example)
  endif()
endif()
