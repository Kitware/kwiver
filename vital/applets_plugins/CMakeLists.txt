kwiver_add_plugin(
  config_explorer
  SUBDIR ${kwiver_plugin_applets_subdir}
  SOURCES register_tools.cxx
          config_explorer.h
          config_explorer.cxx
  PRIVATE kwiversys vital
)
