include(kwiver-test-setup)

add_test(NAME mvg:track_features COMMAND kwiver track-features "${kwiver_test_data_directory}/aphill_240_1fps_crf32.mp4")
