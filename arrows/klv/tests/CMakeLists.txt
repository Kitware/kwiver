project(arrows_test_klv)

set(CMAKE_FOLDER "Arrows/KLV/Tests")

include(kwiver-test-setup)

set( test_libraries kwiver_algo_core vital )

##############################
# KLV tests
##############################

kwiver_discover_gtests( klv klv_read_write LIBRARIES ${test_libraries} )
