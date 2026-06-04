# Build / Install Plugin containing UUID algorithm implementations

set(CMAKE_FOLDER "Arrows/UUID")

set ( plugin_uuid_headers )
set ( plugin_uuid_sources )
set( plugin_uuid_classes
  algo/uuid_factory_uuid
  )

kwiver_expand_class_list( "${plugin_uuid_classes}" plugin_uuid_headers plugin_uuid_sources )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_uuid_export.h
  NOPATH   SUBDIR     arrows/uuid
  )

kwiver_add_library( kwiver_algo_uuid
  ${plugin_uuid_headers}
  ${plugin_uuid_sources}
  )

target_link_libraries( kwiver_algo_uuid
  PUBLIC               vital_algo
  PRIVATE              vital_config
                       ${KWIVER_UUID_LIBRARY}
  )

algorithms_create_plugin( kwiver_algo_uuid
  algo/register_algorithms.cxx
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
