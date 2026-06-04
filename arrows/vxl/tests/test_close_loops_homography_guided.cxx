// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_gtest.h>

#include <arrows/vxl/algo/close_loops_homography_guided.h>
#include <arrows/vxl/algo/image_io.h>

#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <gtest/gtest.h>

using namespace kwiver::vital;
using namespace kwiver::arrows;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( close_loops_homography_guided, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    vxl::close_loops_homography_guided,
    "Use VXL to estimate a sequence of ground plane homographies to identify "
    "frames to match for loop closure.",
    PARAM_DEFAULT(
      enabled, bool,
      "Is long term loop closure enabled?",
      true ),
    PARAM_DEFAULT(
      max_checkpoint_frames, unsigned,
      "Maximum past search distance in terms of number of checkpoints.",
      10000 ),
    PARAM_DEFAULT(
      checkpoint_percent_overlap, double,
      "Term which controls when we make new loop closure checkpoints. "
      "Everytime the percentage of tracked features drops below this "
      "threshold, we generate a new checkpoint.",
      0.7 ),
    PARAM_DEFAULT(
      homography_filename, std::string,
      "Optional output location for a homography text file.",
      "" )
  );
}

// ----------------------------------------------------------------------------
class close_loops_homography_guided : public ::testing::Test
{};

// ----------------------------------------------------------------------------
TEST ( close_loops_homography_guided, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    create_algorithm< algo::close_loops >( "vxl_homography_guided" ) );
}
