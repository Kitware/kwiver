// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/ocv/algo/image_histogram_eq.h>
#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

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
TEST ( image_histogram_eq, create )
{
  kv::plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    kv::create_algorithm< kv::algo::image_filter >( "ocv_histogram_eq" ) );
}

// ----------------------------------------------------------------------------
TEST ( image_histogram_eq, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    image_histogram_eq,
    "Equalize image histogram using OpenCV."
  );
}

// ----------------------------------------------------------------------------
TEST ( image_histogram_eq, check_configuration )
{
  kv::plugin_manager::instance().load_all_plugins();

  auto algo =
    kv::create_algorithm< kv::algo::image_filter >( "ocv_histogram_eq" );
  ASSERT_NE( nullptr, algo );

  auto config = algo->get_configuration();
  EXPECT_TRUE( algo->check_configuration( config ) );
}
