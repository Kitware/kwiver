project(arrows_test_core)

set(CMAKE_FOLDER "Arrows/Core/Tests")

include(kwiver-test-setup)

set( test_libraries    kwiver_algo_core vital vital_vpm )

##############################
# Algorithms core plugin tests
##############################
kwiver_discover_gtests(core csv_io                    LIBRARIES ${test_libraries})
kwiver_discover_gtests(core derive_metadata           LIBRARIES ${test_libraries})
kwiver_discover_gtests(core detected_object_io        LIBRARIES ${test_libraries})
kwiver_discover_gtests(core dynamic_configuration     LIBRARIES ${test_libraries})
kwiver_discover_gtests(core feature_descriptor_io     LIBRARIES ${test_libraries})
kwiver_discover_gtests(core interpolate_track_spline  LIBRARIES ${test_libraries})
kwiver_discover_gtests(core merge_metadata_streams    LIBRARIES ${test_libraries})
kwiver_discover_gtests(core metadata_map_io_csv       LIBRARIES ${test_libraries})
kwiver_discover_gtests(core mesh_intersect            LIBRARIES ${test_libraries})
kwiver_discover_gtests(core mesh_operations           LIBRARIES ${test_libraries})
kwiver_discover_gtests(core metadata_stream_from_video
                                                      LIBRARIES ${test_libraries})
kwiver_discover_gtests(core render_mesh_depth_map     LIBRARIES ${test_libraries})
kwiver_discover_gtests(core track_set_impl            LIBRARIES ${test_libraries})
kwiver_discover_gtests(core transfer_bbox_with_depth_map
                                                      LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(core triangle_scan_iterator    LIBRARIES ${test_libraries})
kwiver_discover_gtests(core uv_unwrap_mesh            LIBRARIES ${test_libraries})
kwiver_discover_gtests(core video_input_filter        LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(core video_input_image_list    LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(core video_input_pos           LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(core video_input_splice        LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(core video_input_split         LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
