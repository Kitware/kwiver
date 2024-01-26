project(arrows_test_ffmpeg)

set(CMAKE_FOLDER "Arrows/FFmpeg/Tests")

include(kwiver-test-setup)

set(test_libraries     kwiver_algo_ffmpeg kwiver_algo_klv kwiver_algo_core)

##############################
# Algorithms ffmpeg tests
##############################
kwiver_discover_gtests(ffmpeg video_input_ffmpeg        LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(ffmpeg video_input_ffmpeg_clip   LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(ffmpeg video_input_ffmpeg_rewire LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(ffmpeg video_output_ffmpeg       LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")

if( KWIVER_ENABLE_SERIALIZE_JSON )
  kwiver_discover_gtests(ffmpeg video_input_ffmpeg_klv
    LIBRARIES
      ${test_libraries}
      kwiver_serialize_json_klv
    ARGUMENTS "${kwiver_test_data_directory}")
endif()
