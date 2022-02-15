# Build / Install plugin containing PDAL algorithm implementations + support
# structures

set(CMAKE_FOLDER "Arrows/PDAL")

set( pdal_headers_public
  write_pdal.h
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
  write_pdal.cxx
  )

kwiver_add_library( kwiver_algo_pdal
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

algorithms_create_plugin( kwiver_algo_pdal
  register_algorithms.cxx
  )

message( STATUS "DIRS ${CMAKE_CURRENT_BINARY_DIR}" )
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/pdal_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/pdal_config.h
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
