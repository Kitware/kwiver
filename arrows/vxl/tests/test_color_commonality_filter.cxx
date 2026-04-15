// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_gtest.h>

#include <arrows/vxl/algo/color_commonality_filter.h>
#include <arrows/vxl/algo/image_io.h>

#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <gtest/gtest.h>

namespace kv = kwiver::vital;
namespace ka = kwiver::arrows;

using namespace kwiver::vital;

kv::path_t g_data_dir;
static std::string test_image_name = "images/kitware_logos/small_grey_logo.png";
static std::string test_color_image_name =
  "images/kitware_logos/small_color_logo.png";
static std::string expected_commonality_default_color =
  "images/kitware_logos/commonality_filter_default_color.png";
static std::string expected_commonality_default_gray =
  "images/kitware_logos/commonality_filter_default_gray.png";

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();

  GET_ARG( 1, g_data_dir );

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class color_commonality_filter : public ::testing::Test
{
  TEST_ARG( data_dir );
};

// ----------------------------------------------------------------------------
TEST_F ( color_commonality_filter, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    ka::vxl::color_commonality_filter,
    "Filter image based on color frequency or commonality.",
    PARAM_DEFAULT(
      color_resolution_per_channel, unsigned,
      "Resolution of the utilized histogram (per channel) if the input "
      "contains 3 channels. Must be a power of two.",
      8 ),
    PARAM_DEFAULT(
      intensity_resolution, unsigned,
      "Resolution of the utilized histogram if the input "
      "contains 1 channel. Must be a power of two.",
      16 ),
    PARAM_DEFAULT(
      output_scale, unsigned,
      "Scale the output image (typically, values start in the range [0,1]) "
      "by this amount. Enter 0 for type-specific default.",
      0 ),
    PARAM_DEFAULT(
      grid_image, bool,
      "Instead of calculating which colors are more common "
      "in the entire image, should we do it for smaller evenly "
      "spaced regions?",
      false ),
    PARAM_DEFAULT(
      grid_resolution_height, unsigned,
      "Divide the height of the image into x regions, if enabled.",
      5 ),
    PARAM_DEFAULT(
      grid_resolution_width, unsigned,
      "Divide the width of the image into x regions, if enabled.",
      6 )
  );
}

// ----------------------------------------------------------------------------
TEST_F ( color_commonality_filter, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    create_algorithm< algo::image_filter >( "vxl_color_commonality" ) );
}

// ----------------------------------------------------------------------------
TEST_F ( color_commonality_filter, color )
{
  ka::vxl::image_io io;

  std::string filename = data_dir + "/" + test_color_image_name;
  auto const image_ptr = io.load( filename );

  ka::vxl::color_commonality_filter filter;

  auto const filtered_image_ptr = filter.filter( image_ptr );

  kv::path_t expected = data_dir + "/" + expected_commonality_default_color;
  auto const expected_image_ptr = io.load( expected );
  EXPECT_TRUE(
    equal_content(
      filtered_image_ptr->get_image(),
      expected_image_ptr->get_image() ) );
}

// ----------------------------------------------------------------------------
TEST_F ( color_commonality_filter, gray )
{
  ka::vxl::image_io io;

  std::string filename = data_dir + "/" + test_image_name;
  auto const image_ptr = io.load( filename );

  ka::vxl::color_commonality_filter filter;

  auto const filtered_image_ptr = filter.filter( image_ptr );

  kv::path_t expected = data_dir + "/" + expected_commonality_default_gray;
  auto const expected_image_ptr = io.load( expected );
  EXPECT_TRUE(
    equal_content(
      filtered_image_ptr->get_image(),
      expected_image_ptr->get_image() ) );
}
