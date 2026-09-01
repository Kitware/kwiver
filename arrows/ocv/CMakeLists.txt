# Build / Install plugin containing OpenCV algorithm implementations + support
# structures

set(CMAKE_FOLDER "Arrows/OCV")

set( plugin_ocv_headers
     bounding_box.h )
set( plugin_ocv_sources
     config_options_helpers.txx )
set( plugin_ocv_classes
  algo/analyze_tracks
  algo/detect_features_AGAST
  algo/detect_features_FAST
  algo/detect_features_GFTT
  algo/detect_features_MSD
  algo/detect_features_MSER
  algo/detect_features_simple_blob
  algo/detect_features_STAR
  algo/detect_heat_map
  algo/detect_motion_3frame_differencing
  algo/detect_motion_mog2
  algo/draw_detected_object_set
  algo/draw_tracks
  algo/estimate_fundamental_matrix
  algo/estimate_homography
  algo/estimate_pnp
  algo/extract_descriptors_BRIEF
  algo/extract_descriptors_DAISY
  algo/extract_descriptors_FREAK
  algo/extract_descriptors_LATCH
  algo/extract_descriptors_LUCID
  algo/feature_detect_extract_BRISK
  algo/feature_detect_extract_ORB
  algo/feature_detect_extract_SIFT
  algo/feature_detect_extract_SURF
  algo/hough_circle_detector
  algo/image_CLAHE
  algo/image_histogram_eq
  algo/image_io
  algo/inpaint
  algo/match_features_bruteforce
  algo/match_features_flannbased
  algo/merge_images
  algo/refine_detections_write_to_disk
  algo/resection_camera
  algo/split_image
  algo/split_image_channels
  algo/detect_features
  algo/extract_descriptors
  algo/match_features
  algo/track_features_klt

  camera_intrinsics
  descriptor_set
  feature_set
  image_container
  mat_image_memory
  match_set
  resection_camera_options
  )

kwiver_expand_class_list( "${plugin_ocv_classes}" plugin_ocv_headers plugin_ocv_sources )

kwiver_install_headers(
  SUBDIR     arrows/ocv
  ${plugin_ocv_headers}
  )
kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_ocv_export.h
  NOPATH   SUBDIR     arrows/ocv
  )

kwiver_add_library( kwiver_algo_ocv
  ${plugin_ocv_headers}
  ${plugin_ocv_sources}
  )
target_compile_definitions( kwiver_algo_ocv
  PUBLIC KWIVER_OPENCV_VERSION_MAJOR=${KWIVER_OPENCV_VERSION_MAJOR}
  )
target_link_libraries( kwiver_algo_ocv
  PUBLIC               vital_algo
                       ${OpenCV_LIBS}
                       kwiver_algo_mvg
  PRIVATE              kwiversys
                       kwiver_algo_core
  )

algorithms_create_plugin( kwiver_algo_ocv
  algo/register_algorithms.cxx
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
