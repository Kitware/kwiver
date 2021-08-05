project(arrows_test_klv)

set(CMAKE_FOLDER "Arrows/KLV/Tests")

include(kwiver-test-setup)

set( test_libraries kwiver_algo_core kwiver_algo_klv vital )

##############################
# KLV tests
##############################

kwiver_discover_gtests( klv klv_checksum   LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_read_write LIBRARIES ${test_libraries} )
