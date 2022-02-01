# Build / Install Key-Length-Value Metadata Arrow

set(CMAKE_FOLDER "Arrows/KLV")

set( sources
  klv_blob.cxx
  klv_checksum.cxx
  klv_convert_vital.cxx
  klv_data_format.cxx
  klv_demuxer.cxx
  klv_key.cxx
  klv_metadata.cxx
  klv_muxer.cxx
  klv_packet.cxx
  klv_read_write.cxx
  klv_set.cxx
  klv_tag_traits.cxx
  klv_timeline.cxx
  klv_value.cxx
  klv_0102.cxx
  klv_0104.cxx
  klv_0601.cxx
  klv_0806.cxx
  klv_0806_aoi_set.cxx
  klv_0806_poi_set.cxx
  klv_0806_user_defined_set.cxx
  klv_1108.cxx
  klv_1108_metric_set.cxx
  misp_time.cxx
  )

set( public_headers
  klv_blob.h
  klv_demuxer.h
  klv_key.h
  klv_metadata.h
  klv_muxer.h
  klv_packet.h
  klv_set.h
  klv_timeline.h
  klv_value.h
  klv_0102.h
  klv_0104.h
  klv_0601.h
  klv_0806.h
  klv_0806_aoi_set.h
  klv_0806_poi_set.h
  klv_0806_user_defined_set.h
  klv_1108.h
  klv_1108_metric_set.h
  misp_time.h
  )

set( private_headers
  klv_data_format.h
  klv_read_write.h
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
