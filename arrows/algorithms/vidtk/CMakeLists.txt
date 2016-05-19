# Build / Install plugin containing vidtk algorithm implementations + support
# structures

set( vidtk_headers_public
  vidtk_change_detector.h
  register_algorithms.h
  )
kwiver_install_headers(
  SUBDIR     arrows/algorithms/vidtk
  ${vidtk_headers_public}
  )
kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/algorithms_vidtk_export.h
  NOPATH   SUBDIR     arrows/algorithms/vidtk
  )

set( vidtk_sources
  vidtk_change_detector.cxx
  register_algorithms.cxx
  )
kwiver_add_library( algorithms_vidtk
  ${vidtk_headers_public}
  ${vidtk_sources}
  )
target_link_libraries( algorithms_vidtk
  PUBLIC               algorithms_vxl  algorithms_core
                       kwiversys       vidtk_object_detectors   )

algorithms_create_plugin( algorithms_vidtk
  algorithm_plugin_interface.cxx
  )

if (ARROWS_ENABLE_TESTS)
  #add_subdirectory(tests)
endif()
