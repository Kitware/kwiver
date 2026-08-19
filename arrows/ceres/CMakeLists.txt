# Build / Install plugin containing Ceres algorithm implementations + support
# structures

set(CMAKE_FOLDER "Arrows/Ceres")

set( ceres_headers_public
  camera_intrinsic_prior.h
  camera_position.h
  camera_smoothness.h
  )
set( ceres_headers_private
  lens_distortion.h
  logging.h
  )

set( ceres_sources
  logging.cxx
  )
set( ceres_classes
  algo/bundle_adjust
  algo/optimize_cameras
  options
  reprojection_error
  types
  )

kwiver_expand_class_list("${ceres_classes}" ceres_headers_public ceres_sources)

kwiver_install_headers(
  SUBDIR     arrows/ceres
  ${ceres_headers_public}
  )

kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/kwiver_algo_ceres_export.h
  NOPATH   SUBDIR     arrows/ceres
  )


kwiver_add_library( kwiver_algo_ceres
  ${ceres_headers_public}
  ${ceres_headers_private}
  ${ceres_sources}
  )
target_link_libraries( kwiver_algo_ceres
  PUBLIC               vital_algo
                       ceres
  PRIVATE              kwiver_algo_mvg
  )

algorithms_create_plugin( kwiver_algo_ceres
  algo/register_algorithms.cxx
  )

if (KWIVER_ENABLE_TESTS)
  add_subdirectory(tests)
endif()
