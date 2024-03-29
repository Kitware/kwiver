# Build / Install plugin containing VXL algorithm implementations + support
# structures

set(CMAKE_FOLDER "Arrows/VXL")

if(NOT KWIVER_ENABLE_KLV)
  message(FATAL_ERROR "The VXL arrow requires the KLV arrow (KWIVER_ENABLE_KLV)")
endif()

set(vxl_headers_public
  aligned_edge_detection.h
  average_frames.h
  bounding_box.h
  bundle_adjust.h
  camera.h
  camera_map.h
  close_loops_homography_guided.h
  color_commonality_filter.h
  compute_homography_overlap.h
  convert_image.h
  estimate_canonical_transform.h
  estimate_essential_matrix.h
  estimate_fundamental_matrix.h
  estimate_homography.h
  estimate_similarity_transform.h
  hashed_image_classifier_filter.h
  high_pass_filter.h
  image_container.h
  image_io.h
  kd_tree.h
  match_features_constrained.h
  morphology.h
  optimize_cameras.h
  pixel_feature_extractor.h
  polygon.h
  split_image.h
  threshold.h
  triangulate_landmarks.h
  vil_image_memory.h
  )

set(vxl_sources
  aligned_edge_detection.cxx
  average_frames.cxx
  bundle_adjust.cxx
  camera.cxx
  camera_map.cxx
  close_loops_homography_guided.cxx
  color_commonality_filter.cxx
  compute_homography_overlap.cxx
  convert_image.cxx
  estimate_canonical_transform.cxx
  estimate_essential_matrix.cxx
  estimate_fundamental_matrix.cxx
  estimate_homography.cxx
  estimate_similarity_transform.cxx
  hashed_image_classifier.cxx
  hashed_image_classifier_filter.cxx
  high_pass_filter.cxx
  image_container.cxx
  image_io.cxx
  kd_tree.cxx
  match_features_constrained.cxx
  morphology.cxx
  optimize_cameras.cxx
  pixel_feature_extractor.cxx
  polygon.cxx
  split_image.cxx
  threshold.cxx
  triangulate_landmarks.cxx
  vil_image_memory.cxx
  )

if( fletch_ENABLE_FFmpeg AND _FFmpeg_version VERSION_LESS 4 )
  list(APPEND vxl_headers_public
    vidl_ffmpeg_video_input.h
  )
  list(APPEND vxl_sources
    vidl_ffmpeg_video_input.cxx
  )
  add_definitions( -DVXL_ENABLE_FFMPEG )
endif()

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
                       vidl
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
