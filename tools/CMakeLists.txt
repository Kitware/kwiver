#
# CMake file for the kwiver tool runner
#

set( sources
  register_tools.cxx

  config_explorer.cxx
  dump_klv.cxx
  )

set( headers
  config_explorer.h
  dump_klv.h
  cxxopts.hpp
  )

###
# Add our plugin path to the default list
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

###
# Add path to applets so plugin loader can find them
if(NOT SKBUILD)
kwiver_make_module_path( ${KWIVER_BINARY_DIR} ${kwiver_plugin_applets} )
kwiver_add_module_path( ${kwiver_module_path_result} )
endif()
###
#
kwiver_add_library( kwiver_tools_applet
               kwiver_applet.h
               kwiver_applet.cxx
)

target_link_libraries( kwiver_tools_applet
    PUBLIC    vital_util
              ${VITAL_BOOST_REGEX}
  )


###
# built the tool runner
kwiver_add_executable( kwiver
           kwiver_tool_runner.cxx
)

target_link_libraries( kwiver
  PRIVATE        vital
                 vital_vpm
                 vital_logger
                 kwiversys
                 kwiver_tools_applet
)
