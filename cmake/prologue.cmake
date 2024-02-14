include("${CMAKE_CURRENT_LIST_DIR}/top.cmake")

set(warning_guard "")
if(NOT PROJECT_IS_TOP_LEVEL)
  option(
      plutovg_INCLUDES_WITH_SYSTEM
      "Use SYSTEM modifier for plutovg's includes, disabling warnings"
      ON
  )
  mark_as_advanced(plutovg_INCLUDES_WITH_SYSTEM)
  if(plutovg_INCLUDES_WITH_SYSTEM)
    set(warning_guard SYSTEM)
  endif()
endif()
