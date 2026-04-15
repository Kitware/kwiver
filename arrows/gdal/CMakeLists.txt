# Build / Install plugin containing GDAL algorithm implementations + support
# structures

set(CMAKE_FOLDER "Arrows/GDAL")

set( gdal_headers_public )
set( gdal_sources )
set( gdal_classes
  algo/image_io
  image_container
  )
kwiver_expand_class_list("${gdal_classes}" gdal_headers_public gdal_sources)

kwiver_install_headers(
  SUBDIR     arrows/gdal
  ${gdal_headers_public}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_gdal_export.h
  NOPATH   SUBDIR     arrows/gdal
  )

kwiver_add_library( kwiver_algo_gdal
  ${gdal_headers_public}
  ${gdal_sources}
  )
target_link_libraries( kwiver_algo_gdal
  PUBLIC               vital_algo
                       ${GDAL_LIBRARY}
  )

algorithms_create_plugin( kwiver_algo_gdal
  register_algorithms.cxx
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
