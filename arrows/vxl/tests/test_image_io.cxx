// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test VXL image class functionality

#include <test_gtest.h>
#include <test_tmpfn.h>

#include <arrows/tests/test_image.h>

#include <arrows/vxl/algo/image_io.h>
#include <arrows/vxl/image_container.h>

#include <kwiversys/SystemTools.hxx>
#include <vital/algo/algorithm.txx>
#include <vital/plugin_management/pluggable_macro_testing.h>
#include <vital/plugin_management/plugin_manager.h>
#include <vital/util/transform_image.h>

#include <gtest/gtest.h>

namespace kv = kwiver::vital;
namespace ka = kwiver::arrows;
using ST = kwiversys::SystemTools;

kv::path_t g_data_dir;
static std::string test_color_image_name =
  "images/kitware_logos/small_color_logo.png";
static std::string test_plane_image_name =
  "images/kitware_logos/planes_logo.png";

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
class image_io : public ::testing::Test
{
  TEST_ARG( data_dir );
};

// ----------------------------------------------------------------------------
TEST_F ( image_io, default_config )
{
  EXPECT_PLUGGABLE_IMPL(
    ka::vxl::image_io,
    "Use VXL (vil) to load and save image files.",
    PARAM_DEFAULT(
      force_byte, bool,
      "When loading, convert the loaded data into a byte "
      "(unsigned char) image regardless of the source data type. "
      "Stretch the dynamic range according to the stretch options "
      "before converting. When saving, convert to a byte image "
      "before writing out the image",
      false ),
    PARAM_DEFAULT(
      auto_stretch, bool,
      "Dynamically stretch the range of the input data such that "
      "the minimum and maximum pixel values in the data map to "
      "the minimum and maximum support values for that pixel "
      "type, or 0.0 and 1.0 for floating point types.  If using "
      "the force_byte option value map between 0 and 255. "
      "Warning, this can result in brightness and constrast "
      "varying between images.",
      false ),
    PARAM_DEFAULT(
      manual_stretch, bool,
      "Manually stretch the range of the input data by "
      "specifying the minimum and maximum values of the data "
      "to map to the full byte range",
      false ),
    PARAM_DEFAULT(
      intensity_range, ka::vxl::array2,
      "The range of intensity values (min, max) to stretch into "
      "the byte range.  This is most useful when e.g. 12-bit "
      "data is encoded in 16-bit pixels. Only used when manual_stretch is "
      "set to true.",
      ka::vxl::array2( { 0, 255 } ) ),
    PARAM_DEFAULT(
      split_channels, bool,
      "When writing out images, if it contains more than one image "
      "plane, write each plane out as a seperate image file. Also, "
      "when enabled at read time, support images written out in via "
      "this method.",
      false )
  );
}

// ----------------------------------------------------------------------------
TEST_F ( image_io, create )
{
  kwiver::vital::plugin_manager::instance().load_all_plugins();

  EXPECT_NE(
    nullptr,
    kwiver::vital::create_algorithm< kwiver::vital::algo::image_io >( "vxl" )
  );
}

// ----------------------------------------------------------------------------
TEST_F ( image_io, save_plane )
{
  // Create an image to output
  auto const vil_image = vil_image_view< vxl_byte >{ 150, 150, 3 };
  auto image = ka::vxl::image_container{ vil_image };
  auto image_ptr = std::make_shared< ka::vxl::image_container >( image );

  // Configure to split channels
  ka::vxl::image_io io;
  auto config = kv::config_block::empty_config();
  config->set_value( "split_channels", true );
  io.set_configuration( config );

  auto const output_filename =
    kwiver::testing::temp_file_name( "image_io_save_plane-", ".png" );

  io.save( output_filename, image_ptr );

  auto const reread_image_ptr = io.load( output_filename );

  EXPECT_TRUE(
    equal_content(
      image_ptr->get_image(),
      reread_image_ptr->get_image() ) );

  auto const saved_filenames = io.plane_filenames( output_filename );

  for( auto const& saved_filename : saved_filenames )
  {
    if( !ST::RemoveFile( saved_filename ) )
    {
      std::cerr << "Failed to remove output vxl plane image" << std::endl;
    }
  }
}

// ----------------------------------------------------------------------------
TEST_F ( image_io, load_plane )
{
  auto const color_filename = data_dir + "/" + test_color_image_name;
  auto const plane_filename = data_dir + "/" + test_plane_image_name;

  ka::vxl::image_io reader;
  auto const color_image_ptr = reader.load( color_filename );

  auto config = kv::config_block::empty_config();
  config->set_value( "split_channels", true );
  reader.set_configuration( config );

  auto const plane_image_ptr = reader.load( plane_filename );
  EXPECT_TRUE(
    equal_content(
      color_image_ptr->get_image(),
      plane_image_ptr->get_image() ) );
}
