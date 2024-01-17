#
# Plugin modules for vital/config components.
#

kwiver_add_plugin( format_config
                   SOURCES          format_config_block_plugin.cxx
                   PRIVATE          vital_config
                                    vital_vpm
                                    vital_util
                                    vital_exceptions
                   SUBDIR           ${kwiver_plugin_module_subdir}
                   )
