#
# Top level build for VITAL
#
project( vital )
set(CMAKE_FOLDER "Vital")

# ==============================================================================
# Vital requirements
# TODO: Eigen (find_package_once? kwiver_find_package?
#       - to allow the creation of standard "depends" like current, but avoid
#         monolithic include at the project root.)

# ==============================================================================
# KWSys
#---------------------------------------------------------------------
# Create the kwsys library for vital.
set(KWSYS_NAMESPACE                kwiversys)
set(KWSYS_USE_DynamicLoader        1)
set(KWSYS_USE_SystemTools          1)
set(KWSYS_USE_System               1)
set(KWSYS_USE_RegularExpression    1)
set(KWSYS_USE_SystemInformation    1)
set(KWSYS_USE_Directory            1)
set(KWSYS_USE_CommandLineArguments 1)
set(KWSYS_USE_MD5 1)

set(KWSYS_INSTALL_EXPORT_NAME  ${kwiver_export_name})
set(KWSYS_INSTALL_INCLUDE_DIR  "include")
set(KWSYS_INSTALL_LIB_DIR      lib)
set(KWSYS_INCLUDE_DIR          ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
set(KWSYS_INSTALL_BIN_DIR      bin)

# Save our custom CXX flags and reset so we pass no special flags to kwiversys.
# Our flags are more strict than expected and causes many warnings and errors.
set( save_flags  ${CMAKE_CXX_FLAGS} )
set( CMAKE_CXX_FLAGS )

include_directories( SYSTEM ${KWSYS_INCLUDE_DIR}/kwiversys )
add_subdirectory(kwiversys)
_kwiver_export( ${KWSYS_NAMESPACE} )
get_target_property(kwsys_target_type "${KWSYS_NAMESPACE}" TYPE)
if (kwsys_target_type STREQUAL "STATIC_LIBRARY")
  _kwiver_compile_pic("${KWSYS_NAMESPACE}")
endif()
set_property(GLOBAL APPEND PROPERTY kwiver_libraries kwiversys)

# force install to standard library directory
# Note that some of the following duplicates what is in kwiver_add_library()
set_target_properties( ${KWSYS_NAMESPACE}
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  INTERFACE_INCLUDE_DIRECTORIES "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/vital/kwiversys>$<INSTALL_INTERFACE:include>"
  )

  if (APPLE)
  set_target_properties( ${KWSYS_NAMESPACE}
    PROPERTIES
    MACOSX_RPATH          TRUE)
else()
  set_target_properties( ${KWSYS_NAMESPACE}
    PROPERTIES
        VERSION          ${KWIVER_VERSION}
        SOVERSION        ${KWIVER_VERSION}
        )
endif()

# restore old CXX flags for our code.
set( CMAKE_CXX_FLAGS ${save_flags} )

# ==============================================================================
# Other VITAL-specific setup and flags

# This regex stuff is to support CentOS with broken std::regex
# - VITAL_USE_STD_REGEX is the product of a compile check, see
#   ``CMake/kwiver-configcheck.cmake`` ~L68.
# - This was introduced for being able to CXXOPTS, found in ``vital/applets/``.
if ( NOT VITAL_USE_STD_REGEX )
  set( VITAL_BOOST_REGEX ${Boost_REGEX_LIBRARY})
  add_definitions( -DKWIVER_USE_BOOST_REGEX )
endif()

# ==============================================================================
# Version header configuration
#
# This file is needed by sprokit so must be generated here, before sprokit is
# included.
#

# Use Git (if available) to add Git hash info to the version header
set(kwiver_configure_with_git on)
kwiver_configure_file(version.h
                      "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
                      "${CMAKE_CURRENT_BINARY_DIR}/version.h"
                      KWIVER_VERSION_MAJOR
                      KWIVER_VERSION_MINOR
                      KWIVER_VERSION_PATCH
                      KWIVER_VERSION
                      KWIVER_SOURCE_DIR
                      )
set(kwiver_configure_with_git)  # Unset modifier variable


# ==============================================================================
#
# This option adds another directory to the path by appending the configuration
# type to existing directories.
#
# TODO: Add rationale on why this option/feature exists. E.g. When is it used?
#
set(default OFF)

if (CMAKE_CONFIGURATION_TYPES)
  set(default ON)
endif ()

option(KWIVER_USE_CONFIGURATION_SUBDIRECTORY
       "Look in the configuration's subdirectory for each module path (e.g. debug, release, ...)" ${default})

# ==============================================================================
# Configure compiler options header
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/vital_config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/vital_config.h" )

# ==============================================================================
# Subdirectory entries
#
# Things here should be in an order that takes into account inter-library
# requirements.
#
# -- Creates library "vital_exceptions"
add_subdirectory( exceptions )
# -- Creates library "vital_util"
add_subdirectory( util )
# -- Creates library "vital_logger"
# Requires: vital_util
add_subdirectory( logger )
# -- Creates library "vital_config"
# Requires: vital_logger, vital_util (vital_types)
add_subdirectory( config )
# -- Creates library "vital_types"
# Requires: vital_config
add_subdirectory( types )
# -- Creates library "vital_vpm"
# Requires: vital_exceptions, vital_logger, vital_util
# Uses common headers: typedefs.h
add_subdirectory( plugin_management )
# -- Created library "vital_applets"
add_subdirectory( applets )

# -- Creates library "vital_algo"
add_subdirectory( algo )

# ==============================================================================
# Vital Library

# "Public" headers are those that are installed.
# NOTE: Some of these pure headers are utilized in the above "dependent"
#       libraries. Don't know what the correct "solution" for this seemingly
#       "cyclical" dependency.
set( vital_public_headers
# Shared pure headers with above "deps"
  bitflags.h
  noncopyable.h
  typedefs.h

  algorithm_capabilities.h

#  vital_types.h
#  attribute_set.h
#  context.h
#  exceptions.h
#  iterator.h
#  math_constants.h
#  optional.h
#  set.h
#  signal.h
#
#  io/camera_from_metadata.h
#  io/camera_io.h
#  io/camera_map_io.h
#  io/eigen_io.h
#  io/landmark_map_io.h
#  io/mesh_io.h
#  io/metadata_io.h
#  io/track_set_io.h
#
#  types/activity.h
#  types/activity_type.h
#  types/bounding_box.h
#  types/camera.h
#  types/camera_intrinsics.h
#  types/camera_map.h
#  types/camera_perspective.h
#  types/camera_perspective_map.h
#  types/camera_rpc.h
#  types/category_hierarchy.h
#  types/class_map.h
#  types/color.h
#  types/covariance.h
#  types/database_query.h
#  types/descriptor.h
#  types/descriptor_request.h
#  types/descriptor_set.h
#  types/detected_object.h
#  types/detected_object_set.h
#  types/detected_object_set_util.h
#  types/detected_object_type.h
#  types/essential_matrix.h
#  types/feature.h
#  types/feature_set.h
#  types/feature_track_set.h
#  types/fundamental_matrix.h
#  types/geo_MGRS.h
#  types/geo_covariance.h
#  types/geo_point.h
#  types/geo_polygon.h
#  types/geodesy.h
#  types/homography.h
#  types/homography_f2f.h
#  types/homography_f2w.h
#  types/image.h
#  types/image_container.h
#  types/image_container_set.h
#  types/image_container_set_simple.h
#  types/iqr_feedback.h
#  types/landmark.h
#  types/landmark_map.h
#  types/local_cartesian.h
#  types/local_geo_cs.h
#  types/match_set.h
#  types/matrix.h
#  types/mesh.h
#  types/metadata.h
#  types/metadata_map.h
#  types/metadata_tags.h
#  types/metadata_traits.h
#  types/metadata_types.h
#  types/object_track_set.h
#  types/point.h
#  types/polygon.h
#  types/query_result.h
#  types/query_result_set.h
#  types/rotation.h
#  types/sfm_constraints.h
#  types/similarity.h
#  types/timestamp.h
#  types/timestamp_config.h
#  types/track.h
#  types/track_descriptor.h
#  types/track_descriptor_set.h
#  types/track_interval.h
#  types/track_set.h
#  types/transform_2d.h
#  types/uid.h
#  types/vector.h
)

# ----------------------
set( vital_sources
  algorithm_capabilities.cxx
#  attribute_set.cxx
#  context.cxx

#  io/camera_from_metadata.cxx
#  io/camera_io.cxx
#  io/camera_map_io.cxx
#  io/landmark_map_io.cxx
#  io/mesh_io.cxx
#  io/metadata_io.cxx
#  io/track_set_io.cxx
#
#  types/activity.cxx
#  types/activity_type.cxx
#  types/bounding_box.cxx
#  types/camera_intrinsics.cxx
#  types/camera_perspective.cxx
#  types/camera_rpc.cxx
#  types/category_hierarchy.cxx
#  types/database_query.cxx
#  types/descriptor.cxx
#  types/descriptor_request.cxx
#  types/descriptor_set.cxx
#  types/detected_object.cxx
#  types/detected_object_set.cxx
#  types/detected_object_set_util.cxx
#  types/detected_object_type.cxx
#  types/essential_matrix.cxx
#  types/feature.cxx
#  types/feature_track_set.cxx
#  types/fundamental_matrix.cxx
#  types/geo_MGRS.cxx
#  types/geo_covariance.cxx
#  types/geo_point.cxx
#  types/geo_polygon.cxx
#  types/geodesy.cxx
#  types/homography.cxx
#  types/homography_f2f.cxx
#  types/homography_f2w.cxx
#  types/image.cxx
#  types/image_container_set_simple.cxx
#  types/iqr_feedback.cxx
#  types/landmark.cxx
#  types/local_cartesian.cxx
#  types/local_geo_cs.cxx
#  types/mesh.cxx
#  types/metadata.cxx
#  types/metadata_traits.cxx
#  types/metadata_types.cxx
#  types/object_track_set.cxx
#  types/point.cxx
#  types/polygon.cxx
#  types/query_result.cxx
#  types/rotation.cxx
#  types/sfm_constraints.cxx
#  types/similarity.cxx
#  types/timestamp.cxx
#  types/track.cxx
#  types/track_descriptor.cxx
#  types/track_set.cxx
#  types/uid.cxx
)

kwiver_install_headers(
  ${vital_public_headers}
  SUBDIR   vital
  )

# install generated headers
kwiver_install_headers(
  ${CMAKE_CURRENT_BINARY_DIR}/vital_export.h
  ${CMAKE_CURRENT_BINARY_DIR}/vital_config.h
  ${CMAKE_CURRENT_BINARY_DIR}/version.h
  SUBDIR   vital
  NOPATH
  )

kwiver_add_library( vital
  ${vital_public_headers}
  ${vital_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/vital_export.h
  ${CMAKE_CURRENT_BINARY_DIR}/vital_config.h
  )

# Make sure the configuration targets flush before building this library.
add_dependencies (vital kwiver_configure)

target_link_libraries( vital
  PRIVATE         kwiversys
  PUBLIC          vital_exceptions
                  vital_util
                  vital_logger
                  vital_config
                  vital_types
                  vital_vpm
                  vital_applets
                  Eigen3::Eigen
  )

# ==============================================================================
# Built-in plugin modules
#
# This is specifically after the Vital library definition as some of these
# require the vital library for parts and utility.
# And, you know, the plugin management stuff (except for the logger, which works
# independently from the main plugin management so that it can have logging
# itself).
#
add_subdirectory( logger_plugins )
add_subdirectory( config_plugins )
add_subdirectory( applets_plugins )

####
##
## This option adds another directory to the path by appending the configuration type to
## existing directories.
#set(default OFF)
#
#if (CMAKE_CONFIGURATION_TYPES)
#  set(default ON)
#endif ()
#
#option(KWIVER_USE_CONFIGURATION_SUBDIRECTORY
#  "Look in the configuration's subdirectory for each module path (e.g. debug, release, ...)" ${default})
#
#
####
##
#add_subdirectory( algo )
#add_subdirectory( range )
#add_subdirectory( bindings )
#
#if (KWIVER_ENABLE_TOOLS)
#  add_subdirectory( applets )
#  add_subdirectory( tools )
#endif()
#
#if (KWIVER_ENABLE_SERIALIZE_PROTOBUF)
#  add_subdirectory( types/protobuf )
#endif()
#
####
## Add tests if enabled
#if (KWIVER_ENABLE_TESTS)
#  add_subdirectory( tests )
#endif()

add_subdirectory( test_interface )
