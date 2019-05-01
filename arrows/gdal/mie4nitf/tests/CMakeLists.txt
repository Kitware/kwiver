project(arrows_test_mie4nitf)

include(kwiver-test-setup)

set(test_libraries kwiver_algo_mie4nitf)

##############################
# Algorithms ffmpeg tests
##############################
kwiver_discover_gtests(mie4nitf mie4nitf_video_input LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
