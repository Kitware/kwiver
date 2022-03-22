include(kwiver-test-setup)

add_test(NAME mvg:applets:track_features
  COMMAND $<CONFIG>/kwiver track-features
            --config "${kwiver_test_data_directory}/config_files/track_features_testing.conf"
            "${kwiver_test_data_directory}/videos/aphill_240_1fps_crf32.mp4")
add_test(NAME mvg:applets:init_cameras_landmarks
  COMMAND $<CONFIG>/kwiver init-cameras-landmarks
            --config "${kwiver_test_data_directory}/config_files/init_cameras_landmarks_testing.conf"
            --video "${kwiver_test_data_directory}/videos/aphill_240_1fps_crf32.mp4"
            --tracks "${kwiver_test_data_directory}/pipeline_data/tracks_light.txt")
set_tests_properties(mvg:applets:init_cameras_landmarks PROPERTIES TIMEOUT 120)
