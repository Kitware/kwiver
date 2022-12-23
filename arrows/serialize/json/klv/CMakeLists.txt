# Build / Install plugin for KLV serialization

if(NOT KWIVER_ENABLE_ZLIB)
  message(FATAL_ERROR "JSON KLV serialization requires the ZLib arrow (KWIVER_ENABLE_ZLIB)")
endif()

set( public_headers
  metadata_map_io.h
 )

set( private_headers
  load_save_klv.h
  )

set( sources
  load_save_klv.cxx
  metadata_map_io.cxx
  )

kwiver_install_headers(
  SUBDIR arrows/serialize/json/klv
  ${public_headers}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_serialize_json_klv_export.h
  NOPATH SUBDIR arrows/serialize/json/klv
  )

kwiver_add_library( kwiver_serialize_json_klv
  ${public_headers}
  ${sources}
  )

target_link_libraries( kwiver_serialize_json_klv
  PUBLIC vital_algo
  PRIVATE
    kwiver_algo_klv
    kwiver_algo_zlib
    kwiver_serialize_json
  )

algorithms_create_plugin( kwiver_serialize_json_klv
  register_algorithms.cxx
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
