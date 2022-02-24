# Build / Install plugin containing PDAL algorithm implementations + support
# structures

set(CMAKE_FOLDER "Arrows/PDAL")

set( pdal_headers_public
  pointcloud_io.h
  )

set( pdal_headers_private
  pdal_config.h
  )

kwiver_install_headers(
  SUBDIR     arrows/pdal
  ${pdal_headers_public}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_pdal_export.h
  NOPATH   SUBDIR     arrows/pdal
  )

set( pdal_sources
  pointcloud_io.cxx
  )

kwiver_add_library( kwiver_algo_pdal SHARED
  ${pdal_headers_public}
  ${pdal_headers_private}
  ${pdal_sources}
  )

target_link_libraries( kwiver_algo_pdal
  PUBLIC               vital_algo
  PRIVATE              ${PDAL_LIBRARIES}
  )

target_include_directories( kwiver_algo_pdal SYSTEM
  PRIVATE                   ${PDAL_INCLUDE_DIRS}
                            ${PDAL_INCLUDE_DIRS}/pdal
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/pdal_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/pdal_config.h
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif(KWIVER_ENABLE_TESTS)
