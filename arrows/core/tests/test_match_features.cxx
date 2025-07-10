// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/core/algo/match_features_fundamental_matrix.h>
#include <arrows/core/algo/match_features_homography.h>
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
TEST ( match_features_fundamental_matrix, create )
{
  using namespace kwiver::vital;

  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    create_algorithm< algo::match_features >( "fundamental_matrix_guided" ) );
}

// ----------------------------------------------------------------------------
TEST ( match_features_homography, create )
{
  using namespace kwiver::vital;

  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr, create_algorithm< algo::match_features >( "homography_guided" ) );
}

// ----------------------------------------------------------------------------
TEST ( match_features_fundamental_matrix, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    match_features_fundamental_matrix,
    "Use an estimated fundamental matrix as a geometric filter"
    " to remove outlier matches.",
    PARAM_DEFAULT(
      inlier_scale, double,
      "The acceptable error distance (in pixels) between a measured point "
      "and its epipolar line to be considered an inlier match.",
      10.0 ),
    PARAM_DEFAULT(
      min_required_inlier_count, int,
      "The minimum required inlier point count. If there are less "
      "than this many inliers, no matches will be returned.",
      0 ),
    PARAM_DEFAULT(
      min_required_inlier_percent, double,
      "The minimum required percentage of inlier points. If the "
      "percentage of points considered inliers is less than this "
      "amount, no matches will be returned.",
      0.0 ),
    PARAM_DEFAULT(
      motion_filter_percentile, double,
      "If less than 1.0, find this percentile of the motion "
      "magnitude and filter matches with motion larger than "
      "twice this value.  This helps remove outlier matches "
      "when the motion between images is small.",
      0.75 ),
    PARAM(
      feature_matcher,
      vital::algo::match_features_sptr,
      "feature_matcher" ),
    PARAM(
      fundamental_matrix_estimator,
      vital::algo::estimate_fundamental_matrix_sptr,
      "fundamental_matrix_estimator" )
  );
}

// ----------------------------------------------------------------------------
TEST ( match_features_homography, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    match_features_homography,
    "Use an estimated homography as a geometric filter"
    " to remove outlier matches.",
    PARAM_DEFAULT(
      inlier_scale, double,
      "The acceptable error distance (in pixels) between warped "
      "and measured points to be considered an inlier match. "
      "Note that this scale is multiplied by the average scale of "
      "the features being matched at each stage.",
      1.0 ),
    PARAM_DEFAULT(
      min_required_inlier_count, int,
      "The minimum required inlier point count. If there are less "
      "than this many inliers, no matches will be output.",
      0 ),
    PARAM_DEFAULT(
      min_required_inlier_percent, double,
      "The minimum required percentage of inlier points. If the "
      "percentage of points considered inliers is less than this "
      "amount, no matches will be output.",
      0.0 ),
    PARAM(
      homography_estimator,
      vital::algo::estimate_homography_sptr,
      "homography_estimator" ),
    PARAM(
      feature_matcher1,
      vital::algo::match_features_sptr,
      "feature_matcher1" ),
    PARAM(
      feature_matcher2,
      vital::algo::match_features_sptr,
      "feature_matcher2" ),
    PARAM(
      filter_features,
      vital::algo::filter_features_sptr,
      "filter_features" )
  );
}
