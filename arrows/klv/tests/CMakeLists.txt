project(arrows_test_klv)

set(CMAKE_FOLDER "Arrows/KLV/Tests")

include(kwiver-test-setup)

set( test_libraries kwiver_algo_core kwiver_algo_klv vital )

##############################
# KLV tests
##############################

kwiver_discover_gtests( klv apply_child_klv     LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_blob            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_checksum        LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_demuxer         LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_muxer           LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_read_write      LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_0102            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_0104            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_0601            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_0806            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_0903            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_1002            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_1010            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_1107            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_1108            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_1108_metric_set LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_1202            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_1206            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_1303            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_1601            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_1602            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv klv_1607            LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv misp_time           LIBRARIES ${test_libraries} )
kwiver_discover_gtests( klv update_klv          LIBRARIES ${test_libraries} )
