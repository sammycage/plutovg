target_sources(plutovg
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-paint.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-matrix.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-path.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-surface.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-canvas.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-rasterize.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-blend.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-ft-raster.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-ft-stroker.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-ft-math.c"
)

target_include_directories(plutovg
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
)
