project(arrows_test_geocalc)

set(CMAKE_FOLDER "Arrows/Geocalc/Tests")

include(kwiver-test-setup)

set( test_libraries kwiver_algo_geocalc vital vital_vpm )

##############################
# Algorithms gepcalc plugin tests
##############################
kwiver_discover_gtests(geocalc derive_corner_points LIBRARIES ${test_libraries})
kwiver_discover_gtests(geocalc geo_conv             LIBRARIES ${test_libraries})
kwiver_discover_gtests(geocalc projection           LIBRARIES ${test_libraries})
