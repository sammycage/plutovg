add_executable(plutovg-fuzz fuzz.c)

#if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#    message(STATUS "Current is ${CMAKE_CXX_COMPILER_ID}")
#    message(FATAL_ERROR "Clang is required for libFuzzer!")
#endif()

target_link_libraries(plutovg-fuzz plutovg)
target_compile_options(plutovg-fuzz PUBLIC -fsanitize=fuzzer)
target_link_options(plutovg-fuzz PUBLIC -fsanitize=fuzzer)

