target_sources(plutovg
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/plutovg.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-paint.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-geometry.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-blend.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-rle.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-stroke.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-ft-raster.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-ft-stroker.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-ft-math.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-stb-png.c"
)

target_include_directories(plutovg
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
)
