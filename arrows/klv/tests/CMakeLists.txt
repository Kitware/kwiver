project(arrows_test_klv)

set(CMAKE_FOLDER "Arrows/KLV/Tests")

include(kwiver-test-setup)

set( test_libraries kwiver_algo_core kwiver_algo_klv vital )

##############################
# KLV tests
##############################

kwiver_discover_gtests( klv klv_blob            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_checksum        LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_demuxer         LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_read_write      LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_0102            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_0104            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_0601            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_1108            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_1108_metric_set LIBRARIES ${test_libraries} )
