# Build / Install Plugin containing core algorithm implementations

set( plugin_core_headers
  register_algorithms.h

  class_probablity_filter.h
  close_loops_bad_frames_only.h
  close_loops_multi_method.h
  compute_ref_homography_core.h
  convert_image_bypass.h
  epipolar_geometry.h
  filter_features_magnitude.h
  hierarchical_bundle_adjust.h
  initialize_cameras_landmarks.h
  interpolate_camera.h
  match_features_homography.h
  match_matrix.h
  metrics.h
  non_maximual_supression.h
  projected_track_set.h
  track_features_core.h
  transform.h
  triangulate.h
  triangulate_landmarks.h
  )
kwiver_install_headers(
  SUBDIR     arrows/core
  ${plugin_core_headers}
  )
kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_export.h
  NOPATH   SUBDIR     arrows/core
  )

set( plugin_core_sources
  register_algorithms.cxx

  class_probablity_filter.cxx
  close_loops_bad_frames_only.cxx
  close_loops_multi_method.cxx
  compute_ref_homography_core.cxx
  convert_image_bypass.cxx
  epipolar_geometry.cxx
  filter_features_magnitude.cxx
  hierarchical_bundle_adjust.cxx
  initialize_cameras_landmarks.cxx
  interpolate_camera.cxx
  match_features_homography.cxx
  match_matrix.cxx
  metrics.cxx
  non_maximual_supression.cxx
  projected_track_set.cxx
  track_features_core.cxx
  transform.cxx
  triangulate.cxx
  triangulate_landmarks.cxx
  )

kwiver_add_library( kwiver_algo
  ${plugin_core_headers}
  ${plugin_core_sources}
  )

target_link_libraries( kwiver_algo
  PUBLIC               vital
  )

algorithms_create_plugin( kwiver_algo
  algorithm_plugin_interface.cxx
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
