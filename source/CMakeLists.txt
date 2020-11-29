target_sources(plutovg
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/plutovg.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-paint.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-geometry.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-blend.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-rle.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-dash.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-font.c"
)

target_include_directories(plutovg
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
)
