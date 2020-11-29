target_sources(plutovg 
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/sw_ft_math.c"
    "${CMAKE_CURRENT_LIST_DIR}/sw_ft_raster.c"
    "${CMAKE_CURRENT_LIST_DIR}/sw_ft_stroker.c"
)

target_include_directories(plutovg
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
)
