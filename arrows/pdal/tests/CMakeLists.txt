project(arrows_test_pdal)

set(CMAKE_FOLDER "Arrows/PDAL/Tests")

include(kwiver-test-setup)

set(test_libraries      kwiver_algo_pdal kwiver_algo_proj )

##############################
# Algorithms PDAL tests
##############################
kwiver_discover_gtests(pdal pointcloud_io  LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
