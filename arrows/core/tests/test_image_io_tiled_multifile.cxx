// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <tests/test_gtest.h>
#include <tests/test_tmpfn.h>

#include <arrows/core/algo/image_io_tiled_multifile.h>

#include <arrows/ocv/algo/image_io.h>

#include <vital/plugin_management/plugin_manager.h>
#include <vital/types/image_container.h>
#include <vital/types/tiled_image_container_simple.h>

using namespace kwiver;
using namespace kwiver::arrows;

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( image_io_tiled_multifile, create )
{
  EXPECT_NE(
    nullptr, vital::create_algorithm<
      vital::algo::image_io >( "tiled_multifile" ) );
}

// ----------------------------------------------------------------------------
TEST ( image_io_tiled_multifile, round_trip )
{
  auto const path = kwiver::testing::temp_file_name( "test-", ".png" );
  core::image_io_tiled_multifile io;
  io.set_image_io( std::make_shared< ocv::image_io >() );

  // Create example image tiles
  auto const tiles =
    std::make_shared< vital::simple_tiled_image_container >(
      256, 256, 4, 4, 3 );
  vital::image image1{
    tiles->tile_width(), tiles->tile_height(), tiles->depth(),
    true, tiles->get_image().pixel_traits() };
  vital::image image2{
    tiles->tile_width(), tiles->tile_height(), tiles->depth(),
    true, tiles->get_image().pixel_traits() };

  // Fill tiles with data
  auto ptr1 = static_cast< uint8_t* >( image1.first_pixel() );
  auto ptr2 = static_cast< uint8_t* >( image2.first_pixel() );
  for( size_t i = 0; i < image1.size(); ++i )
  {
    *ptr1 = i % 256;
    *ptr2 = ( i + 127 ) % 256;
    ++ptr1;
    ++ptr2;
  }

  auto const tile1 =
    std::make_shared< vital::simple_image_container >( image1 );
  auto const tile2 =
    std::make_shared< vital::simple_image_container >( image2 );

  // Assemble tiles into overall image
  tiles->set_tile( 0, 1, tile1 );
  tiles->set_tile( 2, 3, tile2 );

  // Write tiles
  io.save( path, tiles );

  // Ensure test files are removed
  struct _tmp_file_remover
  {
    _tmp_file_remover( std::string const& path ) : path{ path } {}

    ~_tmp_file_remover()
    {
      auto const path1 = path.substr( 0, path.size() - 4 ) + ".0001.0000.png";
      std::remove( path1.c_str() );

      auto const path2 = path.substr( 0, path.size() - 4 ) + ".0003.0002.png";
      std::remove( path2.c_str() );
    }

    std::string path;
  } remover{ path };

  // Read tiles back in
  auto const loaded = io.load( path );
  ASSERT_NE( nullptr, loaded );

  auto const loaded_tiles =
    std::dynamic_pointer_cast< vital::tiled_image_container >( loaded );
  ASSERT_NE( nullptr, loaded_tiles );

  // Check everything is the same as when we wrote it
  ASSERT_EQ( tiles->tile_count(), loaded_tiles->tile_count() );

  auto const loaded_tile1 = loaded_tiles->get_tile( 0, 1 );
  ASSERT_NE( nullptr, loaded_tile1 );
  ASSERT_TRUE(
    vital::equal_content( tile1->get_image(), loaded_tile1->get_image() ) );

  auto const loaded_tile2 = loaded_tiles->get_tile( 2, 3 );
  ASSERT_NE( nullptr, loaded_tile2 );
  ASSERT_TRUE(
    vital::equal_content( tile2->get_image(), loaded_tile2->get_image() ) );
}
