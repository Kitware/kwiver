#
# CMake file for the vital tools
#

set(public_headers
  applet_config.h
  applet_context.h
  config_validation.cxx
  cxxopts.hpp
  kwiver_applet.h
  )

set(source_files
  config_validation.cxx
  kwiver_applet.cxx
  )

###
# tools applet support library
kwiver_add_library( vital_applets
  ${public_headers}
  ${source_files}
)

target_link_libraries( vital_applets
  PUBLIC      ${VITAL_BOOST_REGEX}
              vital_util
              vital_config
              vital_logger
              vital_vpm  # uses pluggable
              kwiversys
  )

###
# Install header files
kwiver_install_headers(
  ${public_headers}
  SUBDIR      vital/applets
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/vital_applets_export.h
  NOPATH
  SUBDIR      vital/applets
  )
