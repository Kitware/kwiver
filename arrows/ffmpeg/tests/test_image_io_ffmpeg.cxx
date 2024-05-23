// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test the FFmpeg image_io implementation

#include <test_tmpfn.h>

#include <arrows/ffmpeg/ffmpeg_image_io.h>
#include <arrows/ffmpeg/tests/common.h>

#include <vital/plugin_loader/plugin_manager.h>

#include <filesystem>
#include <limits>

std::filesystem::path g_data_dir;

namespace {

// ----------------------------------------------------------------------------
template < class T >
kv::image
create_test_image( size_t width, size_t height, size_t depth )
{
  constexpr double maximum = std::numeric_limits< T >::max();
  auto image = kv::image{ width, height, depth };
  for( size_t y = 0; y < height; ++y )
  {
    for( size_t x = 0; x < width; ++x )
    {
      for( size_t c = 0; c < depth; ++c )
      {
        image.at< T >( x, y, c ) =
          static_cast< T >(
            maximum / ( height - 1 ) * y -
            maximum / ( width - 1 ) * x +
            maximum / ( depth - 1 ) * c );
      }
    }
  }
  return image;
}

// ----------------------------------------------------------------------------
template < class T >
void
assert_test_image(
  kv::image const& image, size_t width, size_t height, size_t depth,
  size_t epsilon = 0 )
{
  ASSERT_EQ( width, image.width() );
  ASSERT_EQ( height, image.height() );
  ASSERT_EQ( depth, image.depth() );
  ASSERT_EQ( kv::image_pixel_traits_of< T >(), image.pixel_traits() );

  constexpr double maximum = std::numeric_limits< T >::max();
  for( size_t y = 0; y < height; ++y )
  {
    for( size_t x = 0; x < width; ++x )
    {
      for( size_t c = 0; c < depth; ++c )
      {
        ASSERT_NEAR(
          static_cast< T >(
            maximum / ( height - 1 ) * y -
            maximum / ( width - 1 ) * x +
            maximum / ( depth - 1 ) * c ), image.at< T >( x, y, c ), epsilon );
      }
    }
  }
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
int
main( int argc, char* argv[] )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();

  GET_ARG( 1, g_data_dir );

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( ffmpeg_image_io, create )
{
  EXPECT_NE( nullptr, kv::algo::image_io::create( "ffmpeg" ) );
}

// ----------------------------------------------------------------------------
TEST ( ffmpeg_image_io, load_png )
{
  auto const path = g_data_dir / "images/test.png";

  ffmpeg::ffmpeg_image_io io;
  auto const loaded_image = io.load( path.string() );

  ASSERT_NE( nullptr, loaded_image );

  ASSERT_EQ( 40, loaded_image->height() );
  ASSERT_EQ( 60, loaded_image->width() );
  ASSERT_EQ( 3,  loaded_image->depth() );

  EXPECT_EQ( 0, loaded_image->get_image().at< uint8_t >( 0, 0, 0 ) );
  EXPECT_EQ( 0, loaded_image->get_image().at< uint8_t >( 0, 0, 1 ) );
  EXPECT_EQ( 0, loaded_image->get_image().at< uint8_t >( 0, 0, 2 ) );

  EXPECT_EQ( 0,   loaded_image->get_image().at< uint8_t >( 59, 0, 0 ) );
  EXPECT_EQ( 245, loaded_image->get_image().at< uint8_t >( 59, 0, 1 ) );
  EXPECT_EQ( 0,   loaded_image->get_image().at< uint8_t >( 59, 0, 2 ) );

  EXPECT_EQ( 245, loaded_image->get_image().at< uint8_t >( 59, 39, 0 ) );
  EXPECT_EQ( 0,   loaded_image->get_image().at< uint8_t >( 59, 39, 1 ) );
  EXPECT_EQ( 0,   loaded_image->get_image().at< uint8_t >( 59, 39, 2 ) );

  EXPECT_EQ( 0,   loaded_image->get_image().at< uint8_t >( 0, 39, 0 ) );
  EXPECT_EQ( 0,   loaded_image->get_image().at< uint8_t >( 0, 39, 1 ) );
  EXPECT_EQ( 245, loaded_image->get_image().at< uint8_t >( 0, 39, 2 ) );
}

// ----------------------------------------------------------------------------
TEST ( ffmpeg_image_io, load_jpeg )
{
  auto const path = g_data_dir / "images/test.jpg";

  ffmpeg::ffmpeg_image_io io;
  auto const loaded_image = io.load( path.string() );

  ASSERT_NE( nullptr, loaded_image );

  ASSERT_EQ( 32, loaded_image->height() );
  ASSERT_EQ( 32, loaded_image->width() );
  ASSERT_EQ( 3,  loaded_image->depth() );

  EXPECT_EQ( 0, loaded_image->get_image().at< uint8_t >( 0, 0, 0 ) );
  EXPECT_EQ( 0, loaded_image->get_image().at< uint8_t >( 0, 0, 1 ) );
  EXPECT_EQ( 0, loaded_image->get_image().at< uint8_t >( 0, 0, 2 ) );

  EXPECT_EQ( 1,   loaded_image->get_image().at< uint8_t >( 31, 0, 0 ) );
  EXPECT_EQ( 240, loaded_image->get_image().at< uint8_t >( 31, 0, 1 ) );
  EXPECT_EQ( 0,   loaded_image->get_image().at< uint8_t >( 31, 0, 2 ) );

  EXPECT_EQ( 240, loaded_image->get_image().at< uint8_t >( 31, 31, 0 ) );
  EXPECT_EQ( 0,   loaded_image->get_image().at< uint8_t >( 31, 31, 1 ) );
  EXPECT_EQ( 0,   loaded_image->get_image().at< uint8_t >( 31, 31, 2 ) );

  EXPECT_EQ( 0,   loaded_image->get_image().at< uint8_t >( 0, 31, 0 ) );
  EXPECT_EQ( 0,   loaded_image->get_image().at< uint8_t >( 0, 31, 1 ) );
  EXPECT_EQ( 240, loaded_image->get_image().at< uint8_t >( 0, 31, 2 ) );
}

// ----------------------------------------------------------------------------
TEST ( ffmpeg_image_io, load_tiff )
{
  auto const path = g_data_dir / "images/test.tif";

  ffmpeg::ffmpeg_image_io io;
  auto const loaded_image = io.load( path.string() );

  ASSERT_NE( nullptr, loaded_image );

  ASSERT_EQ( 32, loaded_image->height() );
  ASSERT_EQ( 32, loaded_image->width() );
  ASSERT_EQ( 1,  loaded_image->depth() );

  // This will have to change if / when 16-bit support is added
  EXPECT_EQ( 0,   loaded_image->get_image().at< uint8_t >( 0, 0, 0 ) );
  EXPECT_EQ( 0,   loaded_image->get_image().at< uint8_t >( 31, 0, 0 ) );
  EXPECT_EQ( 239, loaded_image->get_image().at< uint8_t >( 31, 31, 0 ) );
  EXPECT_EQ( 0,   loaded_image->get_image().at< uint8_t >( 0, 31, 0 ) );
}

// ----------------------------------------------------------------------------
TEST ( ffmpeg_image_io, save_png )
{
  auto const path = kwiver::testing::temp_file_name( "test-", ".png" );

  ffmpeg::ffmpeg_image_io io;
  auto const image = create_test_image< uint8_t >( 32, 64, 3 );
  io.save( path, std::make_shared< kv::simple_image_container >( image ) );

  _tmp_file_deleter tmp_file_deleter{ path };

  auto const loaded_image = io.load( path );
  CALL_TEST(
    assert_test_image< uint8_t >, loaded_image->get_image(), 32, 64, 3, 0 );
}

// ----------------------------------------------------------------------------
TEST ( ffmpeg_image_io, save_png_rgba )
{
  auto const path = kwiver::testing::temp_file_name( "test-", ".png" );

  ffmpeg::ffmpeg_image_io io;
  auto const image = create_test_image< uint8_t >( 32, 64, 4 );
  io.save( path, std::make_shared< kv::simple_image_container >( image ) );

  _tmp_file_deleter tmp_file_deleter{ path };

  auto const loaded_image = io.load( path );
  CALL_TEST(
    assert_test_image< uint8_t >, loaded_image->get_image(), 32, 64, 4, 0 );
}

// ----------------------------------------------------------------------------
TEST ( ffmpeg_image_io, save_jpeg )
{
  auto const path = kwiver::testing::temp_file_name( "test-", ".jpg" );

  ffmpeg::ffmpeg_image_io io;

  // Set JPEG to highest quality
  auto config = io.get_configuration();
  config->set_value( "quality", 1 );
  io.set_configuration( config );

  auto const image = create_test_image< uint8_t >( 64, 32, 3 );
  io.save( path, std::make_shared< kv::simple_image_container >( image ) );

  _tmp_file_deleter tmp_file_deleter{ path };

  auto const loaded_image = io.load( path );
  CALL_TEST(
    assert_test_image< uint8_t >, loaded_image->get_image(), 64, 32, 3, 10 );
}

// ----------------------------------------------------------------------------
TEST ( ffmpeg_image_io, save_tiff )
{
  auto const path = kwiver::testing::temp_file_name( "test-", ".tif" );

  ffmpeg::ffmpeg_image_io io;
  auto const image = create_test_image< uint8_t >( 32, 64, 3 );
  io.save( path, std::make_shared< kv::simple_image_container >( image ) );

  _tmp_file_deleter tmp_file_deleter{ path };

  auto const loaded_image = io.load( path );
  CALL_TEST(
    assert_test_image< uint8_t >, loaded_image->get_image(), 32, 64, 3, 0 );
}

// ----------------------------------------------------------------------------
TEST ( ffmpeg_image_io, save_tiff_gray )
{
  auto const path = kwiver::testing::temp_file_name( "test-", ".tif" );

  ffmpeg::ffmpeg_image_io io;
  auto const image = create_test_image< uint8_t >( 32, 64, 1 );
  io.save( path, std::make_shared< kv::simple_image_container >( image ) );

  _tmp_file_deleter tmp_file_deleter{ path };

  auto const loaded_image = io.load( path );
  CALL_TEST(
    assert_test_image< uint8_t >, loaded_image->get_image(), 32, 64, 1, 0 );
}
