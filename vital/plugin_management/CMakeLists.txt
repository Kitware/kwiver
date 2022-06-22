###
# plugin loader

###
# vital plugin manager
set(vital_vpm_headers_public
  pluggable.h
  plugin_factory.h
  plugin_loader.h
  plugin_manager.h
#  plugin_info.h
#  plugin_registrar.h
  )

set(vital_vpm_headers_private
    #  plugin_loader_filter.h
    #  plugin_filter_default.h
    #  plugin_filter_category.h
    #  plugin_manager_internal.h
    ${CMAKE_CURRENT_BINARY_DIR}/vital_vpm_export.h
    )

set(vital_vpm_sources
  plugin_factory.cxx
  plugin_loader.cxx
  plugin_manager.cxx
#  plugin_filter_default.cxx
#  plugin_filter_category.cxx
  )

# This has to be built shared due to the use of the singleton pattern holding
# the central manager instance.
kwiver_add_library(vital_vpm SHARED
  ${vital_vpm_headers_public}
  ${vital_vpm_headers_private}
  ${vital_vpm_sources}
  )

kwiver_install_headers(
  ${vital_vpm_headers_public}
  SUBDIR      vital/plugin_loader
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/vital_vpm_export.h
  SUBDIR      vital/plugin_loader
  NOPATH
  )

target_link_libraries(vital_vpm
  PUBLIC              vital_util
                      kwiversys
  PRIVATE             vital_logger
                      vital_exceptions
                      ${VITAL_BOOST_REGEX}
  )

###
# Add tests if enbled
if (KWIVER_ENABLE_TESTS)
  add_subdirectory( tests )
endif()
