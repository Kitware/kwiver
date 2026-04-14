project(arrows_test_core)

set(CMAKE_FOLDER "Arrows/Core/Tests")

include(kwiver-test-setup)

set( test_libraries    vital_algo vital vital_vpm kwiver_algo_core )

##############################
# Algorithms core plugin tests
##############################
kwiver_discover_gtests(core associate_detections_to_tracks_threshold
                                                      LIBRARIES ${test_libraries})
kwiver_discover_gtests(core class_probability_filter  LIBRARIES ${test_libraries})
kwiver_discover_gtests(core close_loops               LIBRARIES ${test_libraries})
kwiver_discover_gtests(core compute_association_matrix_from_features
                                                      LIBRARIES ${test_libraries})
kwiver_discover_gtests(core compute_ref_homography_core
                                                      LIBRARIES ${test_libraries})
kwiver_discover_gtests(core convert_image_bypass      LIBRARIES ${test_libraries})
kwiver_discover_gtests(core create_detection_grid     LIBRARIES ${test_libraries})
kwiver_discover_gtests(core csv_io                    LIBRARIES ${test_libraries})
kwiver_discover_gtests(core derive_metadata           LIBRARIES ${test_libraries})
kwiver_discover_gtests(core detect_features_filtered  LIBRARIES ${test_libraries})
kwiver_discover_gtests(core detected_object_io        LIBRARIES ${test_libraries})
kwiver_discover_gtests(core detected_object_set_input
                                                      LIBRARIES ${test_libraries})
kwiver_discover_gtests(core detected_object_set_output
                                                      LIBRARIES ${test_libraries})
kwiver_discover_gtests(core dynamic_configuration     LIBRARIES ${test_libraries})
kwiver_discover_gtests(core estimate_canonical_transform
                                                      LIBRARIES ${test_libraries})
kwiver_discover_gtests(core example_detector          LIBRARIES ${test_libraries})
kwiver_discover_gtests(core feature_descriptor_io     LIBRARIES ${test_libraries})
kwiver_discover_gtests(core filter_features           LIBRARIES ${test_libraries})
kwiver_discover_gtests(core filter_tracks             LIBRARIES ${test_libraries})
kwiver_discover_gtests(core handle_descriptor_request_core
                                                      LIBRARIES ${test_libraries})
kwiver_discover_gtests(core image_filter_bypass        LIBRARIES ${test_libraries})
# External image I/O algorithm required to test the tile multifile wrapper
if(KWIVER_ENABLE_OPENCV)
  kwiver_discover_gtests(core image_io_tiled_multifile  LIBRARIES ${test_libraries} kwiver_algo_ocv)
endif()
kwiver_discover_gtests(core initialize_object_tracks_threshold
                                                      LIBRARIES ${test_libraries})
kwiver_discover_gtests(core interpolate_track_spline  LIBRARIES ${test_libraries})
kwiver_discover_gtests(core keyframe_selector_basic   LIBRARIES ${test_libraries})
kwiver_discover_gtests(core match_features            LIBRARIES ${test_libraries})
kwiver_discover_gtests(core match_matrix              LIBRARIES ${test_libraries})
kwiver_discover_gtests(core merge_metadata_streams    LIBRARIES ${test_libraries})
kwiver_discover_gtests(core mesh_intersect            LIBRARIES ${test_libraries})
kwiver_discover_gtests(core mesh_operations           LIBRARIES ${test_libraries})
kwiver_discover_gtests(core metadata_map_io_csv       LIBRARIES ${test_libraries})
kwiver_discover_gtests(core metadata_stream_from_video
                                                      LIBRARIES ${test_libraries})
kwiver_discover_gtests(core read_object_track_set_kw18
                                                      LIBRARIES ${test_libraries})
kwiver_discover_gtests(core read_track_descriptor_set_csv
                                                      LIBRARIES ${test_libraries})

kwiver_discover_gtests(core render_mesh_depth_map     LIBRARIES ${test_libraries})
kwiver_discover_gtests(core texture_mesh              LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(core track_features            LIBRARIES ${test_libraries})
kwiver_discover_gtests(core track_set_impl            LIBRARIES ${test_libraries})
kwiver_discover_gtests(core transfer_bbox_with_depth_map
                                                      LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(core transform_detected_object_set
                                                      LIBRARIES ${test_libraries})
kwiver_discover_gtests(core triangle_scan_iterator    LIBRARIES ${test_libraries})
kwiver_discover_gtests(core uv_unwrap_mesh            LIBRARIES ${test_libraries})
kwiver_discover_gtests(core video_input_buffered_metadata_filter
                                                      LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(core video_input_filter        LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(core video_input_image_list    LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(core video_input_metadata_filter
                                                      LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(core video_input_pos           LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(core video_input_splice        LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(core video_input_split         LIBRARIES ${test_libraries} ARGUMENTS "${kwiver_test_data_directory}")
kwiver_discover_gtests(core write_object_track_set_kw18
                                                      LIBRARIES ${test_libraries})
kwiver_discover_gtests(core write_track_descriptor_set_csv
                                                      LIBRARIES ${test_libraries})
