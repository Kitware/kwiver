# Build / Install plugin for serialization

find_package(Protobuf MODULE REQUIRED)

if(NOT KWIVER_ENABLE_ZLIB)
  message(FATAL_ERROR "Protobuf serialization requires zlib (KWIVER_ENABLE_ZLIB)")
endif()
include_directories( ${ZLIB_INCLUDE_DIRS} )

if(NOT PROTOBUF_FOUND)
  message(FATAL_ERROR "-- Protobuf is required!")
endif()

set( plugin_serialize_protobuf_headers )
set( plugin_serialize_protobuf_sources )
set( plugin_serialize_protobuf_classes
  convert_protobuf
  convert_protobuf_point
  algo/activity
  algo/activity_type
  algo/bounding_box
  algo/detected_object_type
  algo/detected_object
  algo/detected_object_set
  algo/metadata
  algo/timestamp
  algo/image
  algo/string
  algo/track_set
  algo/track_state
  algo/track
  algo/object_track_state
  algo/object_track_set
  )

  kwiver_expand_class_list( "${plugin_serialize_protobuf_classes}" plugin_serialize_headers plugin_serialize_protobuf_sources )

link_directories(${PROTOBUF_LIBRARY})
include_directories(${PROTOBUF_INCLUDE_DIR})

# Need to add this include file to the compile because of a defficiency in the protobuf
# compiler. The compiler has the option to insert an export macro into class definitions
# but has no way of defining that macro. As of libprotoc 2.6.1 this is the accepted way
# of defining the export symbols.
add_compile_options( -include "${CMAKE_BINARY_DIR}/vital/types/protobuf/vital_types_protobuf_export.h" )

kwiver_add_library( kwiver_serialize_protobuf
  ${plugin_serialize_protobuf_headers}
  ${plugin_serialize_protobuf_sources}
  )

target_link_libraries( kwiver_serialize_protobuf
  PUBLIC               vital_algo
  PRIVATE              vital_exceptions
                       vital_types_protobuf
                       ${ZLIB_LIBRARIES}
  )

kwiver_install_headers(
  SUBDIR     arrows/serialization/protobuf
  ${plugin_serialize_protobuf_headers}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_serialize_protobuf_export.h
  NOPATH   SUBDIR     arrows/serialization/protobuf
  )

algorithms_create_plugin( kwiver_serialize_protobuf
  algo/register_algorithms.cxx
  )

if (KWIVER_ENABLE_TESTS)
   add_subdirectory(tests)
endif()
