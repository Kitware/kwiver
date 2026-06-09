# Build / Install Plugin containing core algorithm implementations

set(CMAKE_FOLDER "Arrows/Core")

set( plugin_core_headers )
set( plugin_core_sources )
set( plugin_core_classes
  algo/associate_detections_to_tracks_threshold
  algo/class_probability_filter
  algo/close_loops_appearance_indexed
  algo/close_loops_bad_frames_only
  algo/close_loops_exhaustive
  algo/close_loops_keyframe
  algo/close_loops_multi_method
  algo/compute_association_matrix_from_features
  algo/compute_ref_homography_core
  algo/convert_image_bypass
  algo/create_detection_grid
  algo/derive_metadata
  algo/detect_features_filtered
  algo/detected_object_set_input_csv
  algo/detected_object_set_input_kw18
  algo/detected_object_set_input_simulator
  algo/detected_object_set_output_csv
  algo/detected_object_set_output_kw18
  algo/dynamic_config_none
  algo/estimate_canonical_transform
  algo/example_detector
  algo/feature_descriptor_io
  algo/filter_features_magnitude
  algo/filter_features_nonmax
  algo/filter_features_scale
  algo/filter_tracks
  algo/handle_descriptor_request_core
  algo/image_io_tiled_multifile
  algo/initialize_object_tracks_threshold
  algo/interpolate_track_spline
  algo/keyframe_selector_basic
  algo/match_features_fundamental_matrix
  algo/match_features_homography
  algo/merge_metadata_streams
  algo/metadata_map_io_csv
  algo/read_object_track_set_kw18
  algo/read_track_descriptor_set_csv
  algo/texture_mesh
  algo/track_features_augment_keyframes
  algo/track_features_core
  algo/transfer_bbox_with_depth_map
  algo/transform_detected_object_set
  algo/uv_unwrap_mesh
  algo/video_input_buffered_metadata_filter
  algo/video_input_filter
  algo/video_input_image_list
  algo/video_input_metadata_filter
  algo/video_input_pos
  algo/video_input_splice
  algo/video_input_split
  algo/write_object_track_set_kw18
  algo/write_track_descriptor_set_csv

  colorize
  csv_io
  depth_utils
  match_matrix
  match_tracks
  mesh_intersect
  mesh_operations
  metadata_stream_from_video
  render_mesh_depth_map
  track_set_impl
  triangle_scan_iterator
  )

kwiver_expand_class_list( "${plugin_core_classes}" plugin_core_headers plugin_core_sources )

kwiver_install_headers(
  SUBDIR     arrows/core
  ${plugin_core_headers}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_core_export.h
  NOPATH   SUBDIR     arrows/core
  )

kwiver_add_library( kwiver_algo_core
  ${plugin_core_headers}
  ${plugin_core_sources}
  )

target_link_libraries( kwiver_algo_core
  PUBLIC               vital_algo
  PRIVATE              kwiversys
                       vital_config
                       vital_types
  )

kwiver_add_plugin( kwiver_algo_core_plugin
                   SOURCES          algo/register_algorithms.cxx
                   PRIVATE
                                    vital_vpm
                                    kwiver_algo_core
                   SUBDIR           ${kwiver_plugin_algorithm_subdir}
                   )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()

if (KWIVER_ENABLE_TOOLS)
  add_subdirectory(applets)
endif()
