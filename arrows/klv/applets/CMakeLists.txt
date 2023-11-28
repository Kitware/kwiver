set(sources
  register_applets.cxx

  compare_klv.cxx
  )

set(headers
  compare_klv.h
  )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

kwiver_add_plugin(kwiver_algo_klv_applets
  SUBDIR ${kwiver_plugin_applets_subdir}
  SOURCES ${sources} ${headers}
  PRIVATE
    kwiver_algo_core
    kwiver_algo_klv
    kwiver_tools_applet
  )
