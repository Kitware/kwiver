#
# CMake file for the kwiver tool runner
#

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

kwiver_add_executable( demo_python_impl_call demo_python_impl_call.cxx )
target_link_libraries(
  demo_python_impl_call
  PRIVATE   vital
)
