file(GLOB PLUTOVG_EXAMPLE_FILES "*.c")

foreach(EXAMPLE_FILENAME ${PLUTOVG_EXAMPLE_FILES})
    get_filename_component(EXAMPLE_TARGET ${EXAMPLE_FILENAME} NAME_WE)
    add_executable(${EXAMPLE_TARGET} ${EXAMPLE_FILENAME})
    target_link_libraries(${EXAMPLE_TARGET} plutovg m)
endforeach()
