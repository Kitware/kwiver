# Build / Install Key-Length-Value Metadata Arrow

set(CMAKE_FOLDER "Arrows/KLV")

set( sources
  convert_metadata.cxx
  convert_0601_metadata.cxx
  convert_0104_metadata.cxx
  klv_blob.cxx
  klv_checksum.cxx
  klv_convert_vital.cxx
  klv_data.cxx
  klv_data_format.cxx
  klv_demuxer.cxx
  klv_key.cxx
  klv_packet.cxx
  klv_parse.cxx
  klv_read_write.cxx
  klv_set.cxx
  klv_tag_traits.cxx
  klv_timeline.cxx
  klv_value.cxx
  klv_0104.cxx
  klv_0104_new.cxx
  klv_0601.cxx
  klv_0601_new.cxx
  klv_1108.cxx
  klv_1108_metric_set.cxx
  )

set( public_headers
  convert_metadata.h
  klv_blob.h
  klv_data.h
  klv_demuxer.h
  klv_key.h
  klv_packet.h
  klv_parse.h
  klv_set.h
  klv_timeline.h
  klv_value.h
  klv_0104.h
  klv_0104_new.h
  klv_0601.h
  klv_0601_new.h
  klv_1108.h
  klv_1108_metric_set.h
  misp_time.h
  )

set( private_headers
  klv_data_format.h
  klv_read_write.h
  klv_tag_traits.h
  klv_0601_traits.h
)

kwiver_install_headers(
  ${public_headers}
  SUBDIR   arrows/klv
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_klv_export.h
  NOPATH
  SUBDIR      arrows/klv
  )

kwiver_add_library( kwiver_algo_klv
  ${sources}
  ${public_headers}
  ${private_headers}
  )

target_link_libraries( kwiver_algo_klv
  PUBLIC        vital_algo
  PRIVATE       vital
                vital_logger
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory( tests )
endif()
