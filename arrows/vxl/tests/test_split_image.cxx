// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_gtest.h>

#include <arrows/vxl/algo/split_image.h>

#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <gtest/gtest.h>

namespace kv = kwiver::vital;
namespace ka = kwiver::arrows;

using namespace kwiver::vital;

using kwiver::arrows::vxl::split_image;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( split_image, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE( nullptr, create_algorithm< algo::split_image >( "vxl" ) );
}

// ----------------------------------------------------------------------------
TEST ( split_image, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    split_image,
    "Split a larger image into multiple smaller images" );
}
