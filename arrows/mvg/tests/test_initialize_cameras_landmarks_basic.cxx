// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/mvg/algo/initialize_cameras_landmarks_basic.h>

#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>
#include <vital/types/vector.h>

#include <gtest/gtest.h>

using namespace kwiver::vital;
using namespace kwiver::arrows::mvg;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( initialize_cameras_landmarks_basic, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    initialize_cameras_landmarks_basic,
    "Run SfM to iteratively estimate new cameras and landmarks"
    " using feature tracks.",
    PARAM_DEFAULT(
      verbose, bool,
      "If true, write status messages to the terminal showing "
      "debugging information", false ),

    PARAM_DEFAULT(
      init_from_last, bool,
      "If true, and a camera optimizer is specified, initialize "
      "the camera using the closest exiting camera and optimize", false ),

    PARAM_DEFAULT(
      retriangulate_all, bool,
      "If true, re-triangulate all landmarks observed by a newly "
      "initialized camera.  Otherwise, only triangulate or "
      "re-triangulate landmarks that are marked for initialization.", false ),
    PARAM_DEFAULT(
      reverse_ba_error_ratio, double,
      "After final bundle adjustment, if the Necker reversal of "
      "the solution increases the RMSE by less than this factor, "
      "then run a bundle adjustment on the reversed data and "
      "choose the final solution with the lowest error.  Set to "
      "zero to disable.", 2.0 ),

    PARAM_DEFAULT(
      next_frame_max_distance, size_t,
      "Limit the selection of the next frame to initialize to "
      "within this many frames of an already initialized frame. "
      "If no valid frames are found, double the search range "
      "until a valid frame is found. "
      "A value of zero disables this limit", 0 ),
    PARAM_DEFAULT(
      global_ba_rate, double,
      "Run a global bundle adjustment every time the number of "
      "cameras in the system grows by this multiple.", 1.5 ),

    PARAM_DEFAULT(
      interim_reproj_thresh, double,
      "Threshold for rejecting landmarks based on reprojection "
      "error (in pixels) during intermediate processing steps.", 5.0 ),

    PARAM_DEFAULT(
      final_reproj_thresh, double,
      "Relative threshold for rejecting landmarks based on "
      "reprojection error relative to the median error after "
      "the final bundle adjustment.  For example, a value of 2 "
      "mean twice the median error", 2.0 ),

    PARAM_DEFAULT(
      zoom_scale_thresh, double,
      "Threshold on image scale change used to detect a camera "
      "zoom. If the resolution on target changes by more than "
      "this fraction create a new camera intrinsics model.", 0.1 ),

    PARAM_DEFAULT(
      base_camera_focal_length, double,
      "focal length of the base camera model", 1.0 ),

    PARAM_DEFAULT(
      base_camera_principal_point, vector_2d,
      "The principal point of the base camera model \"x y\".\n"
      "It is usually safe to assume this is the center of the "
      "image.", vector_2d( { 0, 0 } ) ),

    PARAM_DEFAULT(
      base_camera_aspect_ratio, double,
      "the pixel aspect ratio of the base camera model", 1.5 ),

    PARAM_DEFAULT(
      base_camera_skew, double,
      "The skew factor of the base camera model.\n"
      "This is almost always zero in any real camera.", 0.0 ),

    PARAM(
      base_camera, camera_intrinsics_sptr,
      "base camera model parameters group" ),

    // nested algorithm configurations
    PARAM(
      essential_mat_estimator, algo::estimate_essential_matrix_sptr,
      "pointer to the nested algorithm" ),
    PARAM(
      camera_optimizer, algo::optimize_cameras_sptr,
      "pointer to the nested algorithm" ),
    PARAM(
      lm_triangulator, algo::triangulate_landmarks_sptr,
      "pointer to the nested algorithm" ),
    PARAM(
      bundle_adjuster, algo::bundle_adjust_sptr,
      "pointer to the nested algorithm" )
  )
}

// ----------------------------------------------------------------------------
TEST ( initialize_cameras_landmarks_basic, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    create_algorithm< algo::initialize_cameras_landmarks >(
      "mvg-basic" ) );
}
