#
# Python support
#

add_subdirectory(cpp_helpers)

# Define modules with tests in them
set(__testable_modnames
  test_activity
  test_algorithms_common
  test_algorithm_factory
  test_camera
  test_camera_intrinsics
  test_category_hierarchy
  test_config
  test_camera_map
  test_camera_perspective
  test_camera_perspective_map
  test_camera_rpc
  test_color
  test_covariance
  test_database_query
  test_detected_object_set
  # test_detected_object_type
  test_descriptor
  test_descriptor_request
  test_descriptor_set
  test_detected_object
  test_feature
  test_feature_descriptor_io
  test_feature_set
  test_feature_track_set
  test_fundamental_matrix
  test_geo_covariance
  test_geo_MGRS
  test_geo_point
  test_geo_polygon
  test_geodesy
  test_homography
  test_homography_f2f
  test_homography_f2w
  test_image
  test_image_container
  test_image_io
  test_iqr_feedback
  test_landmark
  test_local_cartesian
  test_local_geo_cs
  test_metadata
  test_metadata_map
  test_metadata_tags
  test_metadata_traits
  test_object_track_state
  test_object_track_set
  test_query_result
  test_rotation
  test_point
  test_polygon
  test_sfm_constraints
  test_similarity
  test_timestamp
  test_track
  test_track_descriptor
  test_track_interval
  test_track_set
  test_track_state
  test_transform_2d
  test_transform_2d_io
  test_uid
  )

if(KWIVER_ENABLE_SPROKIT AND KWIVER_ENABLE_ARROWS)
  # DetectedObjectSetInput test requires config bindings in sprokit
  list(APPEND
      __testable_modnames
      test_detected_object_set_input
    )
endif()

# All python filenames_we in the vital.tests module
set(__all_modnames
  __init__
  py_helpers
  ${__testable_modnames}
  )


set(mod_dst)
# may need to somehow grab the location the module is written to
# so we can then have nose run on it - configured to - pyfile_dst of kwiver_configure_file
# Add files to the vital.tests python module
foreach (modname IN LISTS __all_modnames)
  kwiver_add_python_module( ${CMAKE_CURRENT_SOURCE_DIR}/${modname}.py
    vital/tests
    ${modname})
endforeach()

add_subdirectory(alg)
# dont select all tests, just configure a single add test with the name vital_types
# and have nose run, exporting the output to an Xunit xml via commandline args to ctest --
# could probably have set that in the set kwiver_test_runner command or add test...
# here all we should be doing is setting a single nosetest with the path
# to the above generated module

if (KWIVER_ENABLE_TESTS)

  set(NOSE_RUNNER)
  set(modpath vital/tests/)
  set(kwiver_test_working_path)
  set(kwiver_test_output_path)


  include(kwiver-setup-tests-python)
  if (NOSE_RUNNER)

    kwiver_add_nosetests(py_vital)

  endif()

endif()
