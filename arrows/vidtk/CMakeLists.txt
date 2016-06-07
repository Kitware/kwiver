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
  SUBDIR     arrows/vidtk
  ${vidtk_headers_public}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_vidtk_export.h
  NOPATH   SUBDIR     arrows/vidtk
  )

kwiver_add_library( kwiver_algo_vidtk
  ${vidtk_headers_public}
  ${vidtk_sources}
  )

target_link_libraries( kwiver_algo_vidtk
  PUBLIC               kwiver_algo
  PRIVATE              kwiver_algo_vxl
                       vidtk_object_detectors
                       kwiversys
  )

algorithms_create_plugin( kwiver_algo_vidtk
  algorithm_plugin_interface.cxx
  )

if (KWIVER_ENABLE_TESTS)
  #add_subdirectory(tests)
endif()
