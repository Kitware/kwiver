# Build / Install plugin for serialization

if(NOT KWIVER_ENABLE_ZLIB)
  message(FATAL_ERROR "JSON serialization requires zlib (KWIVER_ENABLE_ZLIB)")
endif()
include_directories( ${ZLIB_INCLUDE_DIRS} )

set( plugin_serialize_json_headers)
set( plugin_serialize_json_sources
  load_save.cxx
  load_save_point.cxx
  load_save_metadata.cxx )
set( plugin_serialize_json_classes
  algo/activity
  algo/activity_type
  algo/bounding_box
  algo/detected_object
  algo/detected_object_set
  algo/detected_object_type
  algo/image
  algo/metadata
  algo/metadata_map_io
  algo/object_track_set
  algo/object_track_state
  algo/string
  algo/timestamp
  algo/track
  algo/track_set
  algo/track_state
  )

  kwiver_expand_class_list( "${plugin_serialize_json_classes}"
    plugin_serialize_json_headers plugin_serialize_json_sources )

set( private_headers
  load_save.h
  load_save_point.h
  load_save_track_state.h
  load_save_track_set.h
  track_item.h
  )

kwiver_install_headers(
  SUBDIR     arrows/serialize/json
  ${plugin_serialize_json_headers}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_serialize_json_export.h
  NOPATH   SUBDIR     arrows/serialize/json
  )

kwiver_add_library( kwiver_serialize_json
  ${plugin_serialize_json_headers}
  ${plugin_serialize_json_sources}
  ${private_headers}
  )

target_link_libraries( kwiver_serialize_json
  PUBLIC               vital_algo
  PRIVATE              ${ZLIB_LIBRARIES}
  )

algorithms_create_plugin( kwiver_serialize_json
  algo/register_algorithms.cxx
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()

if (KWIVER_ENABLE_KLV)
  add_subdirectory(klv)
endif()
