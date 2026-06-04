// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <arrows/vxl/algo/estimate_fundamental_matrix.h>

#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <gtest/gtest.h>

using namespace kwiver::vital;
using namespace kwiver::arrows;

using vxl::estimate_fundamental_matrix;

static constexpr double ideal_tolerance = 1e-8; // OCV: 1e-6
static constexpr double outlier_tolerance = 0.02; // OCV: 0.01

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( estimate_fundamental_matrix, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    create_algorithm< algo::estimate_fundamental_matrix >( "vxl" ) );
}

// ----------------------------------------------------------------------------
TEST ( estimate_fundamental_matrix, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    estimate_fundamental_matrix,
    "Use VXL (vpgl) to estimate a fundamental matrix.",
    PARAM_DEFAULT(
      precondition, bool,
      "If true, precondition the data before estimating the "
      "fundamental matrix",
      true ),
    PARAM_DEFAULT(
      method, std::string,
      "Fundamental matrix estimation method to use. "
      "(Note: does not include RANSAC).  Choices are: " +
      estimate_fundamental_matrix::method_converter().element_name_string(),
      "EST_8_POINT" )
  );
}

// ----------------------------------------------------------------------------
#include <arrows/tests/test_estimate_fundamental_matrix.h>
