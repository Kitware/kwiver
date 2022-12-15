# Build / Install STANAG Arrow

set(CMAKE_FOLDER "Arrows/STANAG")

set( sources
  stanag_4607_packet.cxx
  stanag_4607_segments.cxx
  stanag_util.cxx
  )

set( public_headers
  stanag_4607_packet.h
  stanag_4607_segments.h
  stanag_util.h
  )

kwiver_install_headers(
  ${public_headers}
  SUBDIR   arrows/stanag
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_stanag_export.h
  NOPATH
  SUBDIR      arrows/stanag
  )

kwiver_add_library( kwiver_algo_stanag
  ${sources}
  ${public_headers}
  )

target_link_libraries( kwiver_algo_stanag
  PUBLIC        vital_algo
  PRIVATE       vital
                vital_logger
                kwiver_algo_klv kwiversys
  )


#if (KWIVER_ENABLE_TESTS)
#  add_subdirectory( tests )
#endif()
