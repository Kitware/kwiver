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
  aligned_edge_detection
  average_frames
  bundle_adjust
  camera_map
  close_loops_homography_guided
  color_commonality_filter
  compute_homography_overlap
  convert_image
  estimate_canonical_transform
  estimate_essential_matrix
  estimate_fundamental_matrix
  estimate_homography
  estimate_similarity_transform
  hashed_image_classifier
  hashed_image_classifier_filter
  high_pass_filter
  image_container
  image_io
  kd_tree
  match_features_constrained
  morphology
  optimize_cameras
  pixel_feature_extractor
  split_image
  threshold
  triangulate_landmarks
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
