// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_gtest.h>

#include <arrows/vxl/algo/image_io.h>
#include <arrows/vxl/algo/match_features_constrained.h>

#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <gtest/gtest.h>

namespace kv = kwiver::vital;
namespace ka = kwiver::arrows;

using namespace kwiver::vital;

using kwiver::arrows::vxl::match_features_constrained;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( match_features_constrained, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    create_algorithm< algo::match_features >( "vxl_constrained" ) );
}

// ----------------------------------------------------------------------------
TEST ( match_features_constrained, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    match_features_constrained,
    "Use VXL to match descriptors under the constraints of similar geometry "
    "(rotation, scale, position).",
    PARAM_DEFAULT(
      scale_thresh, double,
      "Ratio threshold of scales between matching keypoints (>=1.0)"
      " -1 turns scale thresholding off",
      2 ),
    PARAM_DEFAULT(
      angle_thresh, double,
      "Angle difference threshold between matching keypoints"
      " -1 turns angle thresholding off",
      -1 ),
    PARAM_DEFAULT(
      radius_thresh, double,
      "Search radius for a match in pixels",
      200 )
  );
}
