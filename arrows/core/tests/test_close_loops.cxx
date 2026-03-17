// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/core/algo/close_loops_appearance_indexed.h>
#include <arrows/core/algo/close_loops_bad_frames_only.h>
#include <arrows/core/algo/close_loops_exhaustive.h>
#include <arrows/core/algo/close_loops_keyframe.h>
#include <arrows/core/algo/close_loops_multi_method.h>
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
TEST ( close_loops_appearance_indexed, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr, create_algorithm< algo::close_loops >(
      "appearance_indexed" ) );
}

// ----------------------------------------------------------------------------
TEST ( close_loops_bad_frames_only, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr, create_algorithm< algo::close_loops >(
      "bad_frames_only" ) );
}

// ----------------------------------------------------------------------------
TEST ( close_loops_exhaustive, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr, create_algorithm< algo::close_loops >(
      "exhaustive" ) );
}

// ----------------------------------------------------------------------------
TEST ( close_loops_keyframe, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr, create_algorithm< algo::close_loops >(
      "keyframe" ) );
}

// ----------------------------------------------------------------------------
TEST ( close_loops_multi_method, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr, create_algorithm< algo::close_loops >(
      "multi_method" ) );
}

// ----------------------------------------------------------------------------
TEST ( close_loops_appearance_indexed, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    close_loops_appearance_indexed,
    "Uses bag of words index to close loops.",
    PARAM_DEFAULT(
      min_loop_inlier_matches, size_t,
      "The minimum number of inlier feature matches to accept a loop "
      "connection and join tracks",
      128 ),
    PARAM_DEFAULT(
      geometric_verification_inlier_threshold, double,
      "inlier threshold for fundamental matrix based geometric verification "
      "of loop closure in pixels",
      2.0 ),
    PARAM_DEFAULT(
      max_loop_attempts_per_frame, int,
      "The maximum number of loop closure attempts to make per frame",
      200 ),
    PARAM_DEFAULT(
      tracks_in_common_to_skip_loop_closing, int,
      "If this or more tracks are in common between two frames then don't try "
      "to complete a loop with them",
      0 ),
    PARAM_DEFAULT(
      skip_loop_detection_track_i_over_u_threshold, float,
      "skip loop detection if intersection over union of track ids in two "
      "frames is greater than this",
      0.5f ),
    PARAM_DEFAULT(
      min_loop_inlier_fraction, float,
      "Inlier fraction must be this high to accept a loop completion",
      0.5f ),
    PARAM(
      match_features, vital::algo::match_features_sptr,
      "match_features" ),
    PARAM(
      bag_of_words_matching, vital::algo::match_descriptor_sets_sptr,
      "bag_of_words_matching" ),
    PARAM(
      fundamental_mat_estimator, vital::algo::estimate_fundamental_matrix_sptr,
      "fundamental_mat_estimator" )
  );
}

// ----------------------------------------------------------------------------
TEST ( close_loops_bad_frames_only, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    close_loops_bad_frames_only,
    "Attempts short-term loop closure based on percentage "
    "of feature points tracked.",
    PARAM_DEFAULT(
      enabled, bool,
      "Should bad frame detection be enabled? This option will attempt to "
      "bridge the gap between frames which don't meet certain criteria "
      "(percentage of feature points tracked) and will instead attempt "
      "to match features on the current frame against past frames to "
      "meet this criteria. This is useful when there can be bad frames.",
      true ),
    PARAM_DEFAULT(
      percent_match_req, double,
      "The required percentage of features needed to be matched for a "
      "stitch to be considered successful (value must be between 0.0 and "
      "1.0).",
      0.35 ),
    PARAM_DEFAULT(
      new_shot_length, size_t,
      "Number of frames for a new shot to be considered valid before "
      "attempting to stitch to prior shots.",
      2 ),
    PARAM_DEFAULT(
      max_search_length, size_t,
      "Maximum number of frames to search in the past for matching to "
      "the end of the last shot.",
      5 ),
    PARAM(
      feature_matcher, kwiver::vital::algo::match_features_sptr,
      "feature_matcher" )
  );
}

// ----------------------------------------------------------------------------
TEST ( close_loops_exhaustive, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    close_loops_exhaustive,
    "Exhaustive matching of all frame pairs, "
    "or all frames within a moving window.",
    PARAM_DEFAULT(
      match_req, size_t,
      "The required number of features needed to be matched for a success.",
      100 ),
    PARAM_DEFAULT(
      num_look_back, int,
      "Maximum number of frames to search in the past for matching to "
      "(-1 looks back to the beginning).",
      -1 ),
    PARAM(
      feature_matcher, kwiver::vital::algo::match_features_sptr,
      "feature_matcher" )
  );
}

// ----------------------------------------------------------------------------
TEST ( close_loops_keyframe, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    close_loops_keyframe,
    "Establishes keyframes matches to all keyframes.",
    PARAM_DEFAULT(
      match_req, int,
      "The required number of features needed to be matched for a success.",
      100 ),
    PARAM_DEFAULT(
      search_bandwidth, int,
      "Number of adjacent frames to match to (must be at least 1).",
      10 ),
    PARAM_DEFAULT(
      min_keyframe_misses, size_t,
      "Minimum number of keyframe match misses before creating a new keyframe. "
      "A match miss occurs when the current frame does not match any existing "
      "keyframe (must be at least 1).",
      5 ),
    PARAM_DEFAULT(
      stop_after_match, bool,
      "If set, stop matching additional keyframes after at least "
      "one match is found and then one fails to match.  This "
      "prevents making many comparisons to keyframes that are "
      "likely to fail, but it also misses unexpected matches "
      "that could make the tracks stronger.",
      false ),
    PARAM(
      feature_matcher, kwiver::vital::algo::match_features_sptr,
      "feature_matcher" )
  );
}

// ----------------------------------------------------------------------------
TEST ( close_loops_multi_method, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    close_loops_multi_method,
    "Iteratively run multiple loop closure algorithms.",
    PARAM(
      method, std::vector< vital::algo::close_loops_sptr >,
      "Methods" )
  );
}
