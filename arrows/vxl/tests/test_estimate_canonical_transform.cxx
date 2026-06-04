// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_gtest.h>

#include <arrows/vxl/algo/estimate_canonical_transform.h>
#include <arrows/vxl/algo/image_io.h>

#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <gtest/gtest.h>

namespace kv = kwiver::vital;
namespace ka = kwiver::arrows;

using namespace kwiver::vital;

using kwiver::arrows::vxl::estimate_canonical_transform;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( estimate_canonical_transform, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    create_algorithm< algo::estimate_canonical_transform >( "vxl_plane" ) );
}

// ----------------------------------------------------------------------------
TEST ( estimate_canonical_transform, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    estimate_canonical_transform,
    "Use VXL (rrel) to robustly estimate a ground plane for a canonical transform.",
    PARAM_DEFAULT(
      estimate_scale, bool,
      "Estimate the scale to normalize the data. "
      "If disabled the estimate transform is rigid",
      true ),
    PARAM_DEFAULT(
      trace_level, int,
      "Integer value controlling the verbosity of the "
      "plane search algorithms (0->no output, 3->max output).",
      0 ),
    PARAM_DEFAULT(
      rrel_method, std::string,
      "The robust estimation algorithm to use for plane "
      "fitting. Options are: " +
      estimate_canonical_transform::rrel_converter().element_name_string(),
      "IRLS" ),
    PARAM_DEFAULT(
      desired_prob_good, double,
      "The desired probability of finding the correct plane fit.",
      0.99 ),
    PARAM_DEFAULT(
      max_outlier_frac, double,
      "The maximum fraction of the landmarks that is expected "
      "outliers to the ground plane.",
      0.75 ),
    PARAM_DEFAULT(
      prior_inlier_scale, double,
      "The initial estimate of inlier scale for RANSAC "
      "fitting of the ground plane.",
      0.1 ),
    PARAM_DEFAULT(
      irls_max_iterations, int,
      "The maximum number if iterations when using IRLS",
      15 ),
    PARAM_DEFAULT(
      irls_iterations_for_scale, int,
      "The number of IRLS iterations in which to estimate scale",
      2 ),
    PARAM_DEFAULT(
      irls_conv_tolerance, double,
      "The convergence tolerance for IRLS",
      0.0001 )
  )
}
