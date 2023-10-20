# Build / Install STANAG Arrow

set(CMAKE_FOLDER "Arrows/STANAG")

set( sources
  stanag_4607_packet.cxx
  stanag_util.cxx
  segments/stanag_4607_segments.cxx
  segments/stanag_4607_segment_lookup.cxx
  segments/stanag_4607_mission_segment.cxx
  segments/stanag_4607_dwell_segment.cxx
  )

set( public_headers
  stanag_4607_packet.h
  stanag_util.h
  segments/stanag_4607_segments.h
  segments/stanag_4607_segment_lookup.h
  segments/stanag_4607_mission_segment.h
  segments/stanag_4607_dwell_segment.h
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
