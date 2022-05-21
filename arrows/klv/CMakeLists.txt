# Build / Install Key-Length-Value Metadata Arrow

set(CMAKE_FOLDER "Arrows/KLV")

set( sources
  klv_blob.cxx
  klv_checksum.cxx
  klv_convert_vital.cxx
  klv_data_format.cxx
  klv_demuxer.cxx
  klv_lengthy.cxx
  klv_key.cxx
  klv_metadata.cxx
  klv_muxer.cxx
  klv_packet.cxx
  klv_read_write.cxx
  klv_set.cxx
  klv_tag_traits.cxx
  klv_timeline.cxx
  klv_unimplemented.cxx
  klv_uuid.cxx
  klv_value.cxx
  klv_0102.cxx
  klv_0104.cxx
  klv_0601.cxx
  klv_0806.cxx
  klv_0806_aoi_set.cxx
  klv_0806_poi_set.cxx
  klv_0806_user_defined_set.cxx
  klv_0903.cxx
  klv_0903_algorithm_set.cxx
  klv_0903_location_pack.cxx
  klv_0903_ontology_set.cxx
  klv_0903_vchip_set.cxx
  klv_0903_vfeature_set.cxx
  klv_0903_vmask_set.cxx
  klv_0903_vobject_set.cxx
  klv_0903_vtarget_pack.cxx
  klv_0903_vtrackitem_pack.cxx
  klv_0903_vtrack_set.cxx
  klv_0903_vtracker_set.cxx
  klv_1002.cxx
  klv_1010.cxx
  klv_1108.cxx
  klv_1108_metric_set.cxx
  klv_1202.cxx
  klv_1204.cxx
  klv_1206.cxx
  klv_1303.cxx
  klv_1601.cxx
  klv_1602.cxx
  klv_1607.cxx
  misp_time.cxx
  )

set( public_headers
  klv_all.h
  klv_blob.h
  klv_demuxer.h
  klv_lengthy.h
  klv_list.h
  klv_list.hpp
  klv_key.h
  klv_metadata.h
  klv_muxer.h
  klv_packet.h
  klv_series.h
  klv_set.h
  klv_timeline.h
  klv_unimplemented.h
  klv_value.h
  klv_0102.h
  klv_0104.h
  klv_0601.h
  klv_0806.h
  klv_0806_aoi_set.h
  klv_0806_poi_set.h
  klv_0806_user_defined_set.h
  klv_0903.h
  klv_0903_algorithm_set.h
  klv_0903_location_pack.h
  klv_0903_ontology_set.h
  klv_0903_vchip_set.h
  klv_0903_vfeature_set.h
  klv_0903_vmask_set.h
  klv_0903_vobject_set.h
  klv_0903_vtarget_pack.h
  klv_0903_vtrackitem_pack.h
  klv_0903_vtrack_set.h
  klv_0903_vtracker_set.h
  klv_1002.h
  klv_1010.h
  klv_1108.h
  klv_1108_metric_set.h
  klv_1202.h
  klv_1204.h
  klv_1206.h
  klv_1303.h
  klv_1303.hpp
  klv_1601.h
  klv_1602.h
  klv_1607.h
  misp_time.h
  )

set( private_headers
  klv_data_format.h
  klv_length_value.h
  klv_read_write.h
  klv_util.h
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
