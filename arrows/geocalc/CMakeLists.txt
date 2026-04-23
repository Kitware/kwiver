# Build / Install Geographic Calculation arrow

set(CMAKE_FOLDER "Arrows/Geocalc")

set( plugin_geocalc_headers
  constants.h
  )
set( plugin_geocalc_sources )
set( plugin_geocalc_classes
  algo/derive_corner_points

  geo_conv
  projection
  )

kwiver_expand_class_list( "${plugin_geocalc_classes}" plugin_geocalc_headers plugin_geocalc_sources )

kwiver_install_headers(
  SUBDIR     arrows/geocalc
  ${plugin_geocalc_headers}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_geocalc_export.h
  NOPATH   SUBDIR     arrows/geocalc
  )

kwiver_add_library( kwiver_algo_geocalc
  ${plugin_geocalc_headers}
  ${plugin_geocalc_sources}
  )

target_link_libraries( kwiver_algo_geocalc
  PUBLIC               vital_algo
  PRIVATE              kwiver_algo_core
                       vital_config
  )

algorithms_create_plugin( kwiver_algo_geocalc
  algo/register_algorithms.cxx
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
