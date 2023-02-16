project(arrows_test_zlib)

set(CMAKE_FOLDER "Arrows/Zlib/Tests")

include(kwiver-test-setup)

set( test_libraries kwiver_algo_zlib vital )

##############################
# Zlib tests
##############################

kwiver_discover_gtests(zlib bytestream_compressor LIBRARIES ${test_libraries})
