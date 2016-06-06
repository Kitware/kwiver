# Build / Install plugin containing vidtk algorithm implementations + support
# structures

set( vidtk_sources
  register_algorithms.cxx

  vidtk_change_detector.cxx
  )

set( vidtk_headers_public
  register_algorithms.h
  vidtk_change_detector.h
  )

kwiver_install_headers(
  SUBDIR     arrows/algorithms/vidtk
  ${vidtk_headers_public}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/algorithms_vidtk_export.h
  NOPATH   SUBDIR     arrows/algorithms/vidtk
  )

kwiver_add_library( algorithms_vidtk
  ${vidtk_headers_public}
  ${vidtk_sources}
  )

target_link_libraries( algorithms_vidtk
  PUBLIC               algorithms_core
  PRIVATE              algorithms_vxl
                       vidtk_object_detectors
                       kwiversys
  )

algorithms_create_plugin( algorithms_vidtk
  algorithm_plugin_interface.cxx
  )

if (KWIVER_ENABLE_TESTS)
  #add_subdirectory(tests)
endif()
