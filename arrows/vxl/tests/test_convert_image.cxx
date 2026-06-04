// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_gtest.h>

#include <arrows/vxl/algo/convert_image.h>
#include <arrows/vxl/algo/image_io.h>

#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <gtest/gtest.h>

namespace kv = kwiver::vital;
namespace ka = kwiver::arrows;

using namespace kwiver::vital;

using kwiver::arrows::vxl::convert_image;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( convert_image, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    create_algorithm< algo::image_filter >( "vxl_convert_image" ) );
}

// ----------------------------------------------------------------------------
TEST ( convert_image, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    convert_image,
    "Convert image between different formats or scales.",
    PARAM_DEFAULT(
      format, std::string,
      "Output type format: byte, sbyte, float, double, uint16, uint32, etc.",
      "byte" ),
    PARAM_DEFAULT(
      single_channel, bool,
      "Convert input (presumably multi-channel) to contain a single channel, "
      "using either standard RGB to grayscale conversion weights, or "
      "averaging.",
      false ),
    PARAM_DEFAULT(
      scale_factor, double,
      "Optional input value scaling factor",
      0 ),
    PARAM_DEFAULT(
      random_grayscale, double,
      "Convert input image to a 3-channel grayscale image randomly with this "
      "percentage between 0.0 and 1.0. This is used for machine learning "
      "augmentation.",
      0 ),
    PARAM_DEFAULT(
      percentile_norm, double,
      "If set, between [0, 0.5), perform percentile "
      "normalization such that the output image's min and max "
      "values correspond to the percentiles in the orignal "
      "image at this value and one minus this value, respectively.",
      -1 )
  );
}
