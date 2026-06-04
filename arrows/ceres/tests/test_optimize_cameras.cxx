// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/ceres/algo/optimize_cameras.h>

#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <gtest/gtest.h>

using namespace kwiver::vital;

using kwiver::arrows::ceres::optimize_cameras;
using kwiver::arrows::ceres::LossFunctionType;

static constexpr double noisy_center_tolerance = 1e-8;
static constexpr double noisy_rotation_tolerance = 2e-9;
static constexpr double noisy_intrinsics_tolerance = 2e-6;

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

  EXPECT_NE(
    nullptr,
    kwiver::vital::create_algorithm< kwiver::vital::algo::optimize_cameras >(
      "ceres" ) );
}

TEST ( optimize_cameras, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    optimize_cameras,
    "Uses Ceres Solver to optimize camera parameters",
    PARAM_DEFAULT(
      verbose, bool,
      "If true, write status messages to the terminal showing "
      "optimization progress at each iteration", false ),
    PARAM_DEFAULT(
      loss_function_type, LossFunctionType,
      "Robust loss function type to use.",
      kwiver::arrows::ceres::TRIVIAL_LOSS ),
    PARAM_DEFAULT(
      loss_function_scale, double,
      "Robust loss function scale factor.", 1.0 ),
    PARAM(
      solver_options, solver_options_sptr,
      "pointer to the nested config options for solver" ),
    PARAM(
      camera_options, camera_options_sptr,
      "pointer to the nested config options for camera" )
  );
}

// ----------------------------------------------------------------------------
#include <arrows/tests/test_optimize_cameras.h>
