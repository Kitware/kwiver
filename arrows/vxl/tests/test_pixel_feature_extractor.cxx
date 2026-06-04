// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_gtest.h>

#include <arrows/vxl/algo/image_io.h>
#include <arrows/vxl/algo/pixel_feature_extractor.h>
#include <arrows/vxl/image_container.h>

#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>

#include <vil/vil_plane.h>

#include <gtest/gtest.h>

#include <vector>

namespace kv = kwiver::vital;
namespace ka = kwiver::arrows;

using namespace kv;

kv::path_t g_data_dir;
static std::string test_color_image_name =
  "images/kitware_logos/small_color_logo.png";
static std::string expected_name = "images/kitware_logos/features_expected.png";

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
class pixel_feature_extractor : public ::testing::Test
{
  TEST_ARG( data_dir );
};

// ----------------------------------------------------------------------------
TEST_F ( pixel_feature_extractor, create )
{
  plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    create_algorithm< algo::image_filter >( "vxl_pixel_feature_extractor" ) );
}

// ----------------------------------------------------------------------------
TEST_F ( pixel_feature_extractor, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    ka::vxl::pixel_feature_extractor,
    "Extract various local pixel-wise features from an image.",
    PARAM_DEFAULT(
      enable_color, bool,
      "Enable color channels.",
      true ),
    PARAM_DEFAULT(
      enable_gray, bool,
      "Enable grayscale channel.",
      true ),
    PARAM_DEFAULT(
      enable_aligned_edge, bool,
      "Enable aligned_edge_detection filter.",
      true ),
    PARAM_DEFAULT(
      enable_average, bool,
      "Enable average_frames filter.",
      true ),
    PARAM_DEFAULT(
      enable_color_commonality, bool,
      "Enable color_commonality_filter filter.",
      true ),
    PARAM_DEFAULT(
      enable_high_pass_box, bool,
      "Enable high_pass_filter filter.",
      true ),
    PARAM_DEFAULT(
      enable_high_pass_bidir, bool,
      "Enable high_pass_filter filter.",
      true ),
    PARAM_DEFAULT(
      enable_normalized_variance, bool,
      "Enable the normalized variance since the last shot break. "
      "This will be a scalar multiple with the normal variance until "
      "shot breaks are implemented.",
      true ),
    PARAM_DEFAULT(
      enable_spatial_prior, bool,
      "Enable an image which encodes the location",
      true ),
    PARAM_DEFAULT(
      variance_scale_factor, float,
      "The multiplicative value for the normalized varaince",
      0.32f ),
    PARAM_DEFAULT(
      grid_length, unsigned,
      "The number of grids in each directions of the spatial "
      "prior",
      5 )
  );
}

// ----------------------------------------------------------------------------
TEST_F ( pixel_feature_extractor, compute_all )
{
  std::string input_filename = data_dir + "/" + test_color_image_name;
  std::string expected_filename = data_dir + "/" + expected_name;

  ka::vxl::pixel_feature_extractor filter;
  ka::vxl::image_io io;

  auto const input_image = io.load( input_filename );
  auto const filtered = filter.filter( input_image );

  // Many-plane images are saved in a per-channel format
  auto io_config = kv::config_block::empty_config();
  io_config->set_value( "split_channels", true );
  io.set_configuration( io_config );

  auto const expected = io.load( expected_filename );
  EXPECT_TRUE(
    equal_content(
      filtered->get_image(),
      expected->get_image() ) );
}
