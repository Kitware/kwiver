# Build / Install plugin containing VXL algorithm implementations + support
# structures

set(CMAKE_FOLDER "Arrows/VXL")

if(NOT KWIVER_ENABLE_KLV)
  message(FATAL_ERROR "The VXL arrow requires the KLV arrow (KWIVER_ENABLE_KLV)")
endif()

set(vxl_headers_public
  bounding_box.h
  camera.h
  image_statistics.h
  polygon.h
  )

set( vxl_sources
  camera.cxx
  polygon.cxx
  )

set( vxl_classes
  algo/aligned_edge_detection
  algo/average_frames
  algo/bundle_adjust
  algo/close_loops_homography_guided
  algo/color_commonality_filter
  algo/convert_image
  algo/estimate_canonical_transform
  algo/estimate_essential_matrix
  algo/estimate_fundamental_matrix
  algo/estimate_homography
  algo/estimate_similarity_transform
  algo/hashed_image_classifier_filter
  algo/high_pass_filter
  algo/image_io
  algo/match_features_constrained
  algo/morphology
  algo/optimize_cameras
  algo/pixel_feature_extractor
  algo/split_image
  algo/threshold
  algo/triangulate_landmarks
  camera_map
  compute_homography_overlap
  hashed_image_classifier
  image_container
  kd_tree
  vil_image_memory
  )

kwiver_expand_class_list( "${vxl_classes}" vxl_headers_public vxl_sources )

kwiver_install_headers(
  SUBDIR     arrows/vxl
  ${vxl_headers_public}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_vxl_export.h
  NOPATH   SUBDIR     arrows/vxl
  )

kwiver_add_library( kwiver_algo_vxl
  ${vxl_headers_public}
  ${vxl_sources}
  )
target_link_libraries( kwiver_algo_vxl
  PUBLIC               vital_algo
                       vil vpgl vgl
  PRIVATE              kwiver_algo_klv
                       vital_config
                       kwiver_algo_mvg
                       rrel rsdl
                       vnl
                       vil_algo
                       vgl_algo
                       vpgl_algo
                       kwiversys
  )

algorithms_create_plugin( kwiver_algo_vxl
  register_algorithms.cxx
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
