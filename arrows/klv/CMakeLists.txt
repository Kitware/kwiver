# Build / Install Key-Length-Value Metadata Arrow

set(CMAKE_FOLDER "Arrows/KLV")

set( plugin_klv_headers
  klv_all.h
  klv_list.h
  klv_series.h
  klv_series.hpp
  klv_types.h
  klv_util.h
  klv_1303.hpp
  klv_read_write.txx )
set( plugin_klv_sources )
set( plugin_klv_classes
  algo/apply_child_klv
  algo/update_klv

  klv_blob
  klv_checksum
  klv_convert_vital
  klv_data_format
  klv_demuxer
  klv_lengthy
  klv_length_constraints
  klv_imap
  klv_key
  klv_metadata
  klv_muxer
  klv_packet
  klv_read_write
  klv_set
  klv_stream_settings
  klv_string
  klv_tag_traits
  klv_timeline
  klv_unimplemented
  klv_update_intervals
  klv_update_tracker
  klv_uuid
  klv_value
  klv_0102
  klv_0104
  klv_0601
  klv_0806
  klv_0806_aoi_set
  klv_0806_poi_set
  klv_0806_user_defined_set
  klv_0903
  klv_0903_algorithm_set
  klv_0903_location_pack
  klv_0903_ontology_set
  klv_0903_vchip_set
  klv_0903_vfeature_set
  klv_0903_vmask_set
  klv_0903_vobject_set
  klv_0903_vtarget_pack
  klv_0903_vtrackitem_pack
  klv_0903_vtrack_set
  klv_0903_vtracker_set
  klv_1002
  klv_1010
  klv_1107
  klv_1108
  klv_1108_metric_set
  klv_1202
  klv_1204
  klv_1206
  klv_1303
  klv_1601
  klv_1602
  klv_1607
  misp_time
  )

kwiver_expand_class_list( "${plugin_klv_classes}" plugin_klv_headers plugin_klv_sources )

set( private_headers
  klv_length_value.h
  klv_key_traits.h
)

kwiver_install_headers(
  ${plugin_klv_headers}
  SUBDIR   arrows/klv
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_klv_export.h
  NOPATH
  SUBDIR      arrows/klv
  )

kwiver_add_library( kwiver_algo_klv
  ${plugin_klv_headers}
  ${plugin_klv_sources}
  ${private_headers}
  )

target_link_libraries( kwiver_algo_klv
  PUBLIC        vital_algo
  PRIVATE       vital
                vital_logger
  )

kwiver_add_plugin( kwiver_algo_klv_plugin
                   SOURCES          algo/register_algorithms.cxx
                   PRIVATE
                                    vital_vpm
                                    kwiver_algo_klv
                   SUBDIR           ${kwiver_plugin_algorithm_subdir}
                   )

if (KWIVER_ENABLE_TOOLS)
  add_subdirectory(applets)
endif()

if (KWIVER_ENABLE_TESTS)
  add_subdirectory( tests )
endif()
