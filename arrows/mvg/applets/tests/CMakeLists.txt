include(kwiver-test-setup)

add_test(NAME mvg:applets:track_features
  COMMAND kwiver track-features
            --config "${kwiver_test_data_directory}/track_features_testing.conf"
            "${kwiver_test_data_directory}/aphill_240_1fps_crf32.mp4")
