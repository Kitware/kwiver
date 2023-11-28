#
# vital/types library and headers
#
# This sub-library is intended to be home to canonical typedefs and other
# commonly useful type definitions.
#

set( vital_types_public_headers
  bounding_box.h
  color.h
  geo_polygon.h
  geo_point.h
  geodesy.h
  image.h
  image_container.h
  metadata.h
  metadata_map.h
  metadata_traits.h
  polygon.h
  timestamp.h
  vector.h
  video_raw_image.h
  video_raw_metadata.h
  video_settings.h
  video_uninterpreted_data.h
  )
set( vital_type_sources
  bounding_box.cxx
  geo_polygon.cxx
  geo_point.cxx
  geodesy.cxx
  image.cxx
  metadata.cxx
  metadata_traits.cxx
  polygon.cxx
  timestamp.cxx
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
