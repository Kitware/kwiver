if(fletch_ENABLED_YAMLCPP)
  find_package( yaml-cpp REQUIRED )
else()
  message(STATUS "Cannot build track_oracle KPF support without YAML in Fletch")
  return()
endif()
#
# KPF activities
#

set( track_kpf_activity_public_headers
  track_kpf_activity.h
  file_format_kpf_activity.h
)

set( track_kpf_activity_sources
  track_kpf_activity.cxx
  file_format_kpf_activity.cxx
)

kwiver_install_headers(
  ${track_kpf_activity_public_headers}
  SUBDIR     track_oracle/file_formats/track_kpf_activity
)

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/track_kpf_activity_export.h
  NOPATH SUBDIR     track_oracle/file_formats/track_kpf_activity
)

kwiver_add_library( track_kpf_activity
  ${track_kpf_activity_public_headers}
  ${track_kpf_activity_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/track_kpf_activity_export.h
)

target_link_libraries( track_kpf_activity
  PUBLIC               track_oracle
                       data_terms
                       kwiver_algo_kpf
  PRIVATE              vital_logger
                       vul
                       vgl
                       track_oracle_format_base
                       logging_map
                       kpf_utils
                       ${YAML_CPP_LIBRARIES}
)
