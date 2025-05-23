#
# CMake file for the arrows.mvg.applets
#

set( sources
  register_applets.cxx

  bundle_adjust_tool.cxx
  init_cameras_landmarks.cxx
  track_features.cxx
  )

set( headers
  bundle_adjust_tool.h
  init_cameras_landmarks.h
  track_features.h
  )

###
# Add our plugin path to the default list
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()

# Add applet plugin
kwiver_add_plugin( kwiver_algo_mvg_applets
  SUBDIR       ${kwiver_plugin_applets_subdir}
  SOURCES      ${sources} ${headers}
  PRIVATE      kwiver_algo_core
               kwiver_algo_mvg
               kwiver_tools_applet
               kwiversys
)
