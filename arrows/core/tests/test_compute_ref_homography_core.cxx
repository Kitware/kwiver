// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/core/algo/compute_ref_homography_core.h>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <gtest/gtest.h>

#include <algorithm>

using namespace kwiver::vital;
using namespace kwiver::arrows::core;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( compute_ref_homography_core, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr, create_algorithm< algo::compute_ref_homography >( "core" ) );
}

// ----------------------------------------------------------------------------
TEST ( compute_ref_homography_core, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    compute_ref_homography_core,
    "Default online sequential-frame reference homography estimator.",
    PARAM_DEFAULT(
      use_backproject_error,
      bool,
      "Should we remove extra points if the backproject error is high?",
      false ),
    PARAM_DEFAULT(
      backproject_threshold_sqr,
      double,
      "Backprojection threshold in terms of L2 distance squared "
      "(number of pixels)",
      16.0 ),
    PARAM_DEFAULT(
      forget_track_threshold,
      size_t,
      "After how many frames should we forget all info about a track?",
      5 ),
    PARAM_DEFAULT(
      min_track_length,
      size_t,
      "Minimum track length to use for homography regression",
      1 ),
    PARAM_DEFAULT(
      inlier_scale,
      double,
      "The acceptable error distance (in pixels) between warped "
      "and measured points to be considered an inlier match.",
      2.0 ),
    PARAM_DEFAULT(
      minimum_inliers, size_t,
      "Minimum number of matches required between source and "
      "reference planes for valid homography estimation.",
      4 ),
    PARAM_DEFAULT(
      allow_ref_frame_regression, bool,
      "Allow for the possibility of a frame, N, to have a "
      "reference frame, A, when a frame M < N has a reference frame B > A "
      "(assuming frames were sequentially iterated over with this algorithm).",
      true )
  );
}
