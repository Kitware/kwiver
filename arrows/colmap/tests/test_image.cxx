#include <arrows/colmap/colmap.h>
#include <colmap/mvs/image.h>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <test_gtest.h>
#include <vital/algo/image_io.h>
#include <vital/plugin_loader/plugin_manager.h>
#include <vital/types/image.h>
#include <vital/types/image_container.h>

std::string g_data_dir;
std::string test_image_name = "/test_kitware_logo.jpg";

TEST ( colmap, vital_to_colmap )
{
  // TODO: Compare each attribute individually and check for equality
}

TEST ( colmap, vital_to_bitmap )
{
  std::string data_dir = g_data_dir + test_image_name;

  // load test image in colmap
  colmap::Bitmap expected_img;
  expected_img.Allocate( 2370, 1927, true );
  expected_img.Read( data_dir );

  // load test image from vital
  kwiver::vital::algo::image_io_sptr ocv_io =
    kwiver::vital::algo::image_io::create( "ocv" );
  kwiver::vital::image_container_sptr vital_img = ocv_io->load( data_dir );

  // convert vital to colmap
  colmap::Bitmap actual_img =
    kwiver::arrows::colmap_arrow::image_container::vital_to_bitmap(
       vital_img->get_image() );

  // compare vital and colmap images
  std::vector< uint8_t > expected_data = expected_img.ConvertToRowMajorArray();
  colmap::Bitmap col_vital_img;
  col_vital_img.Allocate(2370, 1927, true);
  actual_img.Write( "./logo2.png", FIF_PNG, PNG_DEFAULT );
  col_vital_img.Read("./logo2.png");
  for(int j = 0; j < 1927; j++)
  {
    for(int i = 0; i < 2370; i++)
    {
      // Compare every pixel
      colmap::BitmapColor<uint8_t> col_vital_pixel;
      colmap::BitmapColor<uint8_t> actual_pixel;
      col_vital_img.GetPixel(j, i, &col_vital_pixel);
      actual_img.GetPixel(j, i, &actual_pixel);

      EXPECT_EQ(col_vital_pixel, actual_pixel);
    }
  }
}

int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );

  kwiver::vital::plugin_manager::instance().load_all_plugins();

  GET_ARG( 1, g_data_dir );

  return RUN_ALL_TESTS();
}
