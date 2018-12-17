project(sprokit_tests_test)

include(kwiver-test-setup)

set(test_libraries  sprokit_pipeline kwiversys )

kwiver_discover_gtests(test test_libraries LIBRARIES ${test_libraries}) 

##
##
##

set(test_libraries
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${CMAKE_DL_LIBS}
  )

kwiver_discover_gtests(dlopen test_libraries LIBRARIES ${test_libraries})

