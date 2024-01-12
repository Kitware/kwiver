#
# vital/types library and headers
#
# This sub-library is intended to be home to canonical typedefs and other
# commonly useful type definitions.
#

set( vital_types_public_headers
  activity.h
  activity_type.h
  bounding_box.h
  camera.h
  camera_intrinsics.h
  camera_map.h
  camera_perspective.h
  camera_perspective_map.h
  camera_rpc.h
  category_hierarchy.h
  class_map.h
  color.h
  covariance.h
  database_query.h
  descriptor.h
  descriptor_request.h
  descriptor_set.h
  detected_object.h
  detected_object_set.h
  detected_object_set_util.h
  detected_object_type.h
  essential_matrix.h
  feature.h
  feature_set.h
  feature_track_set.h
  fundamental_matrix.h
  geo_covariance.h
  geodesy.h
  geo_MGRS.h
  geo_point.h
  geo_polygon.h
  ground_control_point.h
  homography_f2f.h
  homography_f2w.h
  homography.h
  image_container.h
  image_container_set.h
  image_container_set_simple.h
  image.h
  iqr_feedback.h
  landmark.h
  landmark_map.h
  local_cartesian.h
  local_geo_cs.h
  match_set.h
  matrix.h
  mesh.h
  metadata.h
  metadata_map.h
  metadata_stream_from_map.h
  metadata_stream.h
  metadata_tags.h
  metadata_traits.h
  object_track_set.h
  pointcloud.h
  point.h
  polygon.h
  query_result.h
  query_result_set.h
  rotation.h
  sfm_constraints.h
  similarity.h
  timestamp_config.h
  timestamp.h
  track_descriptor.h
  track_descriptor_set.h
  track.h
  track_interval.h
  track_set.h
  transform_2d.h
  uid.h
  vector.h
  video_raw_image.h
  video_raw_metadata.h
  video_settings.h
  video_uninterpreted_data.h
  )
set( vital_type_sources
  activity.cxx
  activity_type.cxx
  bounding_box.cxx
  camera_intrinsics.cxx
  camera_perspective.cxx
  camera_rpc.cxx
  category_hierarchy.cxx
  database_query.cxx
  descriptor.cxx
  descriptor_request.cxx
  descriptor_set.cxx
  detected_object.cxx
  detected_object_set.cxx
  detected_object_set_util.cxx
  detected_object_type.cxx
  essential_matrix.cxx
  feature.cxx
  feature_track_set.cxx
  fundamental_matrix.cxx
  geo_covariance.cxx
  geodesy.cxx
  geo_MGRS.cxx
  geo_point.cxx
  geo_polygon.cxx
  ground_control_point.cxx
  homography.cxx
  homography_f2f.cxx
  homography_f2w.cxx
  image_container_set_simple.cxx
  image.cxx
  iqr_feedback.cxx
  landmark.cxx
  local_cartesian.cxx
  local_geo_cs.cxx
  mesh.cxx
  metadata.cxx
  metadata_stream.cxx
  metadata_stream_from_map.cxx
  metadata_traits.cxx
  object_track_set.cxx
  pointcloud.cxx
  point.cxx
  polygon.cxx
  query_result.cxx
  rotation.cxx
  sfm_constraints.cxx
  similarity.cxx
  timestamp.cxx
  track.cxx
  track_descriptor.cxx
  track_set.cxx
  uid.cxx
  video_raw_image.cxx
  video_raw_metadata.cxx
  video_settings.cxx
  video_uninterpreted_data.cxx
  )

kwiver_install_headers(
  ${vital_types_public_headers}
  SUBDIR vital/types
)
kwiver_install_headers(  # generated export header file
  ${CMAKE_CURRENT_BINARY_DIR}/vital_types_export.h
  SUBDIR vital/types
  NOPATH
)
kwiver_add_library(
  vital_types
  ${vital_types_public_headers}
  ${vital_type_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/vital_types_export.h
  )

# Depend on file configuration
add_dependencies( vital_types kwiver_configure )

target_link_libraries( vital_types
  PUBLIC    Eigen3::Eigen
            vital_config
  )
