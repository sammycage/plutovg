include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(
    DIRECTORY include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT plutovg_Development
)

install(
    TARGETS plutovg
    EXPORT plutovgTargets
    DESTINATION dummy
    ARCHIVE #
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    COMPONENT plutovg_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    plutovgConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    plutovg_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/plutovg"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE plutovg_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(plutovg_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${plutovg_INSTALL_CMAKEDIR}"
    RENAME plutovgConfig.cmake
    COMPONENT plutovg_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/plutovgConfigVersion.cmake"
    DESTINATION "${plutovg_INSTALL_CMAKEDIR}"
    COMPONENT plutovg_Development
)

install(
    EXPORT plutovgTargets
    NAMESPACE plutovg::
    DESTINATION "${plutovg_INSTALL_CMAKEDIR}"
    COMPONENT plutovg_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
