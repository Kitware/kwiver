#
# adapter process tests
#

add_executable( basic_test   basic_test.cxx )

target_link_libraries( basic_test
  PRIVATE        sprokit_pipeline sprokit_tools
                 kwiver_adapter
                 ${Boost_SYSTEM_LIBRARY}
  )
