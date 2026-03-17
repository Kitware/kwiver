// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/mvg/algo/hierarchical_bundle_adjust.h>

#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

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
TEST ( hierarchical_bundle_adjust, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    hierarchical_bundle_adjust,
    "Run a bundle adjustment algorithm in a temporally hierarchical fashion"
    " (useful for video)",
    PARAM_DEFAULT(
      initial_sub_sample, size_t,
      "Sub-sample the given cameras by this factor. Gaps will "
      "then be filled in by iterations of interpolation.", 1 ),

    PARAM_DEFAULT(
      interpolation_rate, size_t,
      "Number of cameras to fill in each iteration. When this "
      "is set to 0, we will interpolate all missing cameras "
      "at the first moment possible.", 0 ),

    PARAM_DEFAULT(
      rmse_reporting_enabled, bool,
      "Enable the reporting of RMSE statistics at various "
      "stages of this algorithm. Constant calculating of RMSE "
      "may effect run time of the algorithm.", false ),
    PARAM(
      sba_impl, vital::algo::bundle_adjust_sptr,
      "pointer to the nested algorithm" ),
    PARAM(
      camera_optimizer, vital::algo::optimize_cameras_sptr,
      "pointer to the nested algorithm" ),
    PARAM(
      lm_triangulator, vital::algo::triangulate_landmarks_sptr,
      "pointer to the nested algorithm" )
  )
}

// ----------------------------------------------------------------------------
TEST ( hierarchical_bundle_adjust, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    create_algorithm< algo::bundle_adjust >( "mvg" ) );
}
