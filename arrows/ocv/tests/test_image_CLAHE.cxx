// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/ocv/algo/image_CLAHE.h>
#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>
#include <vital/types/image_container.h>

#include <gtest/gtest.h>

namespace kv = kwiver::vital;
using namespace kwiver::arrows::ocv;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( image_CLAHE, create )
{
  kv::plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr, kv::create_algorithm< kv::algo::image_filter >( "ocv_CLAHE" ) );
}

// ----------------------------------------------------------------------------
TEST ( image_CLAHE, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    image_CLAHE,
    "Adaptively equalize image contrast using OpenCV CLAHE." );
}

// ----------------------------------------------------------------------------
TEST ( image_CLAHE, check_configuration_valid )
{
  kv::plugin_manager::instance().load_all_plugins();

  auto algo = kv::create_algorithm< kv::algo::image_filter >( "ocv_CLAHE" );
  ASSERT_NE( nullptr, algo );

  auto config = algo->get_configuration();
  EXPECT_TRUE( algo->check_configuration( config ) );
}

// ----------------------------------------------------------------------------
TEST ( image_CLAHE, check_configuration_invalid_clip_limit )
{
  kv::plugin_manager::instance().load_all_plugins();

  auto algo = kv::create_algorithm< kv::algo::image_filter >( "ocv_CLAHE" );
  ASSERT_NE( nullptr, algo );

  auto config = algo->get_configuration();
  config->set_value( "clip_limit", -1.0 );
  EXPECT_FALSE( algo->check_configuration( config ) );
}

// ----------------------------------------------------------------------------
TEST ( image_CLAHE, check_configuration_invalid_tile_size )
{
  kv::plugin_manager::instance().load_all_plugins();

  auto algo = kv::create_algorithm< kv::algo::image_filter >( "ocv_CLAHE" );
  ASSERT_NE( nullptr, algo );

  auto config = algo->get_configuration();
  config->set_value( "tile_grid_width", 0 );
  EXPECT_FALSE( algo->check_configuration( config ) );
}
