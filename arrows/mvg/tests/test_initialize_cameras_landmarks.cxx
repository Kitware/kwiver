// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/mvg/algo/initialize_cameras_landmarks.h>

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
TEST ( initialize_cameras_landmarks, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    initialize_cameras_landmarks,
    "Run SfM to estimate new cameras and landmarks "
    "using feature tracks.",
    PARAM_DEFAULT(
      verbose, bool,
      "If true, write status messages to the terminal showing "
      "debugging information", false ),

    PARAM_DEFAULT(
      force_common_intrinsics, bool,
      "If true, then all cameras will share a single set of camera "
      "intrinsic parameters", true ),

    PARAM_DEFAULT(
      frac_frames_for_init, double,
      "fraction of keyframes used in relative pose initialization", -1.0 ),

    PARAM_DEFAULT(
      min_frame_to_frame_matches, size_t,
      "Minimum number of frame-to-frame feature matches "
      "required to attempt reconstruction", 100 ),

    PARAM_DEFAULT(
      interim_reproj_thresh, double,
      "Threshold for rejecting landmarks based on reprojection "
      "error (in pixels) during intermediate processing steps.", 10.0 ),

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

    PARAM_DEFAULT(
      max_cams_in_keyframe_init, int,
      "the maximum number of cameras to reconstruct in "
      "initialization step before switching to resectioning "
      "remaining cameras.", 20 ),

    PARAM_DEFAULT(
      metadata_init_permissive_triang_thresh, double,
      "threshold to apply to triangulation in the first "
      "permissive rounds of metadata based reconstruction "
      "initialization", 10000.0 ),

    // double ang_thresh_cur = acos(m_priv->m_thresh_triang_cos_ang) *
    // rad_to_deg;
    PARAM_DEFAULT(
      feature_angle_threshold, double,
      "feature must have this triangulation angle to keep, in degrees", 2.0 ),

    PARAM_DEFAULT(
      do_final_sfm_cleaning, bool,
      "run a final sfm solution cleanup when solution is complete", false ),
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
      "pointer to the nested algorithm" ),
    PARAM(
      global_bundle_adjuster, algo::bundle_adjust_sptr,
      "pointer to the nested algorithm" ),
    PARAM(
      estimate_pnp, algo::estimate_pnp_sptr,
      "pointer to the nested algorithm" ),
    PARAM(
      canonical_estimator, algo::estimate_canonical_transform_sptr,
      "pointer to the nested algorithm" ),
    PARAM(
      similarity_estimator, algo::estimate_similarity_transform_sptr,
      "pointer to the nested algorithm" )
  )
}

// ----------------------------------------------------------------------------
TEST ( initialize_cameras_landmarks, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    create_algorithm< algo::initialize_cameras_landmarks >( "mvg" ) );
}
