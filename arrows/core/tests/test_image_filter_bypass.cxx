// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/core/algo/image_filter_bypass.h>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>
#include <vital/types/image_container.h>

#include <gtest/gtest.h>

namespace kv = kwiver::vital;
using namespace kwiver::arrows::core;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( image_filter_bypass, create )
{
  kv::plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    kv::create_algorithm< kv::algo::image_filter >( "bypass" ) );
}

// ----------------------------------------------------------------------------
TEST ( image_filter_bypass, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    image_filter_bypass,
    "Performs no filtering and returns the given image container."
  );
}

// ----------------------------------------------------------------------------
TEST ( image_filter_bypass, filter_returns_input )
{
  kv::plugin_manager::instance().load_all_plugins();

  auto algo = kv::create_algorithm< kv::algo::image_filter >( "bypass" );
  ASSERT_NE( nullptr, algo );

  auto img = std::make_shared< kv::simple_image_container >( kv::image{} );
  auto result = algo->filter( img );

  EXPECT_EQ( img, result );
}
