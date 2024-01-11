kwiver_add_plugin(
  video_input
  SUBDIR ${kwiver_plugin_algo_subdir}
  SOURCES 
          #register_tools.cxx
          video_input.h
          video_input.cxx
  PRIVATE vital
)
