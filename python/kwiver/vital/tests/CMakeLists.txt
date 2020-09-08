#
# Python support
#

# Define modules with tests in them
set(__testable_modnames
  test_algorithms_common
  test_algorithm_factory
  test_camera
  test_camera_intrinsics
  test_config
  test_camera_map
  test_color
  test_covariance
  test_descriptor
  test_descriptor_set
  test_eigen_numpy
  test_feature
  test_feature_descriptor_io
  test_homography
  test_image
  test_image_container
  test_image_io
  test_landmark
  test_object_track_state
  test_object_track_set
  test_rotation
  test_similarity
  test_timestamp
  test_track
  test_track_set
  test_track_state
  test_transform_2d_io
  )

# All python filenames_we in the vital.tests module
set(__all_modnames
  helpers
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
