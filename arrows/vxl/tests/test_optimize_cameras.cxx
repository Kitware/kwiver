// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/vxl/algo/optimize_cameras.h>

#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <gtest/gtest.h>

using namespace kwiver::vital;

using kwiver::arrows::vxl::optimize_cameras;

static constexpr double noisy_center_tolerance = 2e-10;
static constexpr double noisy_rotation_tolerance = 2e-10;
static constexpr double noisy_intrinsics_tolerance = 2e-10;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( optimize_cameras, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE( nullptr, create_algorithm< algo::optimize_cameras >( "vxl" ) );
}

// ----------------------------------------------------------------------------
TEST ( optimize_cameras, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    optimize_cameras,
    "Use VXL (vpgl) to optimize camera parameters for fixed "
    "landmarks and tracks.",
  );
}

// ----------------------------------------------------------------------------
#include <arrows/tests/test_optimize_cameras.h>
