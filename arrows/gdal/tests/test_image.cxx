// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test GDAL image class

#include <test_gtest.h>
#include <test_tmpfn.h>

#include <arrows/gdal/algo/image_io.h>
#include <arrows/tests/test_image.h>
#include <vital/plugin_management/plugin_manager.h>
#include <vital/types/geodesy.h>
#include <vital/types/metadata.h>
#include <vital/types/metadata_traits.h>

kwiver::vital::path_t g_data_dir;

namespace algo = kwiver::vital::algo;
namespace gdal = kwiver::arrows::gdal;
namespace vital = kwiver::vital;


static int expected_size = 32;
static std::string geotiff_file_name = "images/test.tif";
static std::string nitf_file_name = "images/test.ntf";
static std::string jpeg_file_name = "images/test.jpg";
static std::string png_file_name = "images/test.png";
static std::vector< int > test_x_pixels = { 0, 3, 11, 21, 31 };
static std::vector< int > test_y_pixels = { 0, 5, 8, 13, 31 };

static std::vector< kwiver::vital::vital_metadata_tag > rpc_tags = {
  kwiver::vital::VITAL_META_RPC_HEIGHT_OFFSET,
  kwiver::vital::VITAL_META_RPC_HEIGHT_SCALE,
  kwiver::vital::VITAL_META_RPC_LONG_OFFSET,
  kwiver::vital::VITAL_META_RPC_LONG_SCALE,
  kwiver::vital::VITAL_META_RPC_LAT_OFFSET,
  kwiver::vital::VITAL_META_RPC_LAT_SCALE,
  kwiver::vital::VITAL_META_RPC_ROW_OFFSET,
  kwiver::vital::VITAL_META_RPC_ROW_SCALE,
  kwiver::vital::VITAL_META_RPC_COL_OFFSET,
  kwiver::vital::VITAL_META_RPC_COL_SCALE,
  kwiver::vital::VITAL_META_RPC_ROW_NUM_COEFF,
  kwiver::vital::VITAL_META_RPC_ROW_DEN_COEFF,
  kwiver::vital::VITAL_META_RPC_COL_NUM_COEFF,
  kwiver::vital::VITAL_META_RPC_COL_DEN_COEFF, };

static std::vector< kwiver::vital::vital_metadata_tag > nitf_tags = {
  kwiver::vital::VITAL_META_NITF_IDATIM,
  kwiver::vital::VITAL_META_NITF_BLOCKA_FRFC_LOC_01,
  kwiver::vital::VITAL_META_NITF_BLOCKA_FRLC_LOC_01,
  kwiver::vital::VITAL_META_NITF_BLOCKA_LRLC_LOC_01,
  kwiver::vital::VITAL_META_NITF_BLOCKA_LRFC_LOC_01,
  kwiver::vital::VITAL_META_NITF_IMAGE_COMMENTS, };

// ----------------------------------------------------------------------------
int
main( int argc, char* argv[] )
{
  ::testing::InitGoogleTest( &argc, argv );

  GET_ARG( 1, g_data_dir );

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
void
test_rpc_metadata( kwiver::vital::metadata_sptr md )
{
  for( auto const& tag : rpc_tags )
  {
    EXPECT_TRUE( md->has( tag ) )
      << "Image metadata should include "
      << kwiver::vital::tag_traits_by_tag( tag ).name();
  }

  if( md->size() > 0 )
  {
    std::cout << "-----------------------------------\n" << std::endl;
    kwiver::vital::print_metadata( std::cout, *md );
  }
}

// ----------------------------------------------------------------------------
void
test_nitf_metadata( kwiver::vital::metadata_sptr md )
{
  for( auto const& tag : nitf_tags )
  {
    EXPECT_TRUE( md->has( tag ) )
      << "Image metadata should include "
      << kwiver::vital::tag_traits_by_tag( tag ).name();
  }

  if( md->size() > 0 )
  {
    kwiver::vital::print_metadata( std::cout, *md );
  }
}

// ----------------------------------------------------------------------------
// This has to return by reference because it has ASSERTs in it, which return
// void on failure.
void
save_load_format(
  std::string const& extension, std::string const& data_dir,
  vital::metadata_sptr const& metadata,
  vital::image_container_sptr& out_img_container )
{
  gdal::image_io image_io;
  vital::path_t png_filepath = data_dir + "/" + png_file_name;
  auto const png_img_container = image_io.load( png_filepath );
  ASSERT_NE( nullptr, png_img_container );

  png_img_container->set_metadata( metadata );


  auto const out_filepath =
    kwiver::testing::temp_file_name( "test-", extension );
  image_io.save( out_filepath, png_img_container );

  out_img_container = image_io.load( out_filepath );
  std::remove( out_filepath.c_str() );
  ASSERT_NE( nullptr, out_img_container );


  auto const png_img = png_img_container->get_image();
  auto const out_img = out_img_container->get_image();

  ASSERT_EQ( png_img.width(), out_img.width() );
  ASSERT_EQ( png_img.height(), out_img.height() );
  ASSERT_EQ( png_img.depth(), out_img.depth() );
  ASSERT_EQ( png_img.pixel_traits(), out_img.pixel_traits() );
  ASSERT_EQ( out_img_container->pixel_traits(), out_img.pixel_traits() );

  EXPECT_TRUE( vital::equal_content( png_img, out_img ) );
}

// ----------------------------------------------------------------------------
class image_io : public ::testing::Test
{
  TEST_ARG( data_dir );
};

// ----------------------------------------------------------------------------
TEST_F ( image_io, create )
{
  kwiver::vital::plugin_manager::instance().load_all_plugins();

  ASSERT_NE(
    nullptr,
    kwiver::vital::create_algorithm< algo::image_io >( "gdal" ) );
}

TEST_F ( image_io, load_geotiff )
{
  kwiver::arrows::gdal::image_io img_io;

  kwiver::vital::path_t file_path = data_dir + "/" + geotiff_file_name;
  auto img_ptr = img_io.load( file_path );

  EXPECT_EQ( img_ptr->width(), expected_size );
  EXPECT_EQ( img_ptr->height(), expected_size );
  EXPECT_EQ( img_ptr->depth(), 1 );


  // Test some pixel values
  kwiver::vital::image_of< uint16_t > img( img_ptr->get_image() );
  for( auto x_px : test_x_pixels )
  {
    for( auto y_px : test_y_pixels )
    {
      uint16_t expected_pixel_value = ( std::numeric_limits< uint16_t >::max() +
                                        1 ) *
                                      x_px * y_px / expected_size /
                                      expected_size;
      EXPECT_EQ( img( x_px, y_px ), expected_pixel_value );
    }
  }


  auto md = img_ptr->get_metadata();

  test_rpc_metadata( md );

  // Test corner points
  ASSERT_TRUE( md->has( kwiver::vital::VITAL_META_CORNER_POINTS ) )
    << "Metadata should include corner points.";


  auto corner_pts = md->find( kwiver::vital::VITAL_META_CORNER_POINTS )
    .get< kwiver::vital::geo_polygon >();
  EXPECT_EQ( corner_pts.crs(), 4326 );
  EXPECT_TRUE( corner_pts.polygon( 4326 ).contains( -16.0, 0.0 ) );
  EXPECT_TRUE( corner_pts.polygon( 4326 ).contains( 0.0, 32.0 ) );
  EXPECT_TRUE( corner_pts.polygon( 4326 ).contains( 0.0, -32.0 ) );
  EXPECT_TRUE( corner_pts.polygon( 4326 ).contains( 16.0, 0.0 ) );
}

TEST_F ( image_io, load_nitf )
{
  kwiver::arrows::gdal::image_io img_io;

  kwiver::vital::path_t file_path = data_dir + "/" + nitf_file_name;
  auto img_ptr = img_io.load( file_path );

  EXPECT_EQ( img_ptr->width(), expected_size );
  EXPECT_EQ( img_ptr->height(), expected_size );
  EXPECT_EQ( img_ptr->depth(), 1 );


  // Test some pixel values
  kwiver::vital::image_of< float > img( img_ptr->get_image() );
  for( auto x_px : test_x_pixels )
  {
    for( auto y_px : test_y_pixels )
    {
      float expected_pixel_value = x_px * y_px /
                                   float( expected_size * expected_size );
      EXPECT_EQ( img( x_px, y_px ), expected_pixel_value );
    }
  }


  auto md = img_ptr->get_metadata();

  test_rpc_metadata( md );
}

TEST_F ( image_io, load_nitf_2 )
{
  kwiver::arrows::gdal::image_io img_io;
  kwiver::vital::path_t file_path = data_dir + "/" + nitf_file_name;
  auto img_ptr = img_io.load( file_path );

  EXPECT_EQ( img_ptr->width(), 32 );
  EXPECT_EQ( img_ptr->height(), 32 );
  EXPECT_EQ( img_ptr->depth(), 1 );


  auto md = img_ptr->get_metadata();
  test_nitf_metadata( md );
}

TEST_F ( image_io, load_jpeg )
{
  kwiver::arrows::gdal::image_io img_io;

  kwiver::vital::path_t file_path = data_dir + "/" + jpeg_file_name;
  auto img_ptr = img_io.load( file_path );

  EXPECT_EQ( img_ptr->width(), expected_size );
  EXPECT_EQ( img_ptr->height(), expected_size );
  EXPECT_EQ( img_ptr->depth(), 3 );


  uint8_t norm_fact =
    expected_size * expected_size /
    ( std::numeric_limits< uint8_t >::max() + 1 );

  // Test some pixel values
  kwiver::vital::image_of< uint8_t > img( img_ptr->get_image() );
  for( auto x_px : test_x_pixels )
  {
    for( auto y_px : test_y_pixels )
    {
      auto pixel = img.at( x_px, y_px );

      uint8_t expected_red = x_px * y_px / norm_fact;
      uint8_t expected_blue = ( expected_size - x_px - 1 ) * y_px / norm_fact;
      uint8_t expected_green = x_px * ( expected_size - y_px - 1 ) / norm_fact;
      // Due to lossy compression exact comparisons will fail
      EXPECT_NEAR( pixel.r, expected_red, 1 )
        << "Incorrect red value at pixel (" << x_px << "," << y_px << ")";
      EXPECT_NEAR( pixel.b, expected_blue, 1 )
        << "Incorrect blue value at pixel (" << x_px << "," << y_px << ")";
      EXPECT_NEAR( pixel.g, expected_green, 1 )
        << "Incorrect green value at pixel (" << x_px << "," << y_px << ")";
    }
  }
}

// ----------------------------------------------------------------------------
TEST_F ( image_io, save_load_nitf_blocka )
{
  auto metadata = std::make_shared< vital::metadata >();
  metadata->add< vital::VITAL_META_NITF_BLOCKA_FRFC_LOC_01 >(
    "+45.123456-045.123456" );
  metadata->add< vital::VITAL_META_NITF_BLOCKA_FRLC_LOC_01 >(
    "-00.123456+145.223456" );
  metadata->add< vital::VITAL_META_NITF_BLOCKA_LRLC_LOC_01 >(
    "S001122.33E1795959.99" );
  metadata->add< vital::VITAL_META_NITF_BLOCKA_LRFC_LOC_01 >(
    "N000000.01W0051234.56" );


  vital::image_container_sptr nitf_img_container;
  save_load_format( ".nitf", data_dir, metadata, nitf_img_container );
  ASSERT_NE( nullptr, nitf_img_container );


  auto const nitf_metadata = nitf_img_container->get_metadata();
  ASSERT_NE( nullptr, nitf_metadata );


  auto const corner_points_entry =
    nitf_metadata->find( vital::VITAL_META_CORNER_POINTS );
  ASSERT_TRUE( corner_points_entry );


  auto const corner_points =
    corner_points_entry.get< vital::geo_polygon >().polygon().get_vertices();
  ASSERT_EQ( 4, corner_points.size() );

  for( auto const& [ i, tag, lat, lon ] : {
    std::make_tuple(
      0, vital::VITAL_META_NITF_BLOCKA_FRFC_LOC_01, +45.123456, -045.123456 ),
    std::make_tuple(
      1, vital::VITAL_META_NITF_BLOCKA_FRLC_LOC_01, -00.123456, +145.223456 ),
    std::make_tuple(
      2, vital::VITAL_META_NITF_BLOCKA_LRLC_LOC_01, -00.189536, +179.999997 ),
    std::make_tuple(
      3, vital::VITAL_META_NITF_BLOCKA_LRFC_LOC_01, +00.000003, -005.209600 ),
  } )
  {
    EXPECT_EQ(
      metadata->find( tag ).get< std::string >(),
      nitf_metadata->find( tag ).get< std::string >() ) << i;


    auto const& corner = corner_points[ i ];
    EXPECT_NEAR( lon, corner[ 0 ], 5e-7 ) << i;
    EXPECT_NEAR( lat, corner[ 1 ], 5e-7 ) << i;
  }
}

// ----------------------------------------------------------------------------
TEST_F ( image_io, save_load_nitf_corners_no_blocka )
{
  auto metadata = std::make_shared< vital::metadata >();
  std::vector< vital::vector_2d > vertices = {
    { -045.123456, +45.123456 },
    { +145.223456, -00.123456 },
    { +179.999997, -00.189536 },
    { -005.209600, +00.000003 }, };
  vital::geo_polygon polygon{ vertices, vital::SRID::lat_lon_WGS84 };
  metadata->add< vital::VITAL_META_CORNER_POINTS >( polygon );


  vital::image_container_sptr nitf_img_container;
  save_load_format( ".nitf", data_dir, metadata, nitf_img_container );
  ASSERT_NE( nullptr, nitf_img_container );


  auto const nitf_metadata = nitf_img_container->get_metadata();
  ASSERT_NE( nullptr, nitf_metadata );


  auto const corner_points_entry =
    nitf_metadata->find( vital::VITAL_META_CORNER_POINTS );
  ASSERT_TRUE( corner_points_entry );


  auto const corner_points =
    corner_points_entry.get< vital::geo_polygon >().polygon().get_vertices();
  ASSERT_EQ( 4, corner_points.size() );

  for( size_t i = 0; i < 4; ++i )
  {
    auto const& vertex = vertices[ i ];
    auto const& corner = corner_points[ i ];
    EXPECT_NEAR( vertex[ 0 ], corner[ 0 ], 5e-7 ) << i;
    EXPECT_NEAR( vertex[ 1 ], corner[ 1 ], 5e-7 ) << i;
  }
}

// ----------------------------------------------------------------------------
TEST_F ( image_io, save_load_geotiff )
{
  auto metadata = std::make_shared< vital::metadata >();
  std::vector< vital::vector_2d > vertices = {
    { -45.123456, +45.123456 },
    { -45.223456, +45.123456 },
    { -45.223456, +45.223456 },
    { -45.123456, +45.223456 }, };
  vital::geo_polygon polygon{ vertices, vital::SRID::lat_lon_WGS84 };
  metadata->add< vital::VITAL_META_CORNER_POINTS >( polygon );


  vital::image_container_sptr geotiff_img_container;
  save_load_format( ".tif", data_dir, metadata, geotiff_img_container );
  ASSERT_NE( nullptr, geotiff_img_container );


  auto const geotiff_metadata = geotiff_img_container->get_metadata();
  ASSERT_NE( nullptr, geotiff_metadata );


  auto const corner_points_entry =
    geotiff_metadata->find( vital::VITAL_META_CORNER_POINTS );
  ASSERT_TRUE( corner_points_entry );


  auto const corner_points =
    corner_points_entry.get< vital::geo_polygon >().polygon().get_vertices();
  ASSERT_EQ( 4, corner_points.size() );

  for( size_t i = 0; i < 4; ++i )
  {
    auto const& vertex = vertices[ i ];
    auto const& corner = corner_points[ i ];
    EXPECT_NEAR( vertex[ 0 ], corner[ 0 ], 1e-15 ) << i;
    EXPECT_NEAR( vertex[ 1 ], corner[ 1 ], 1e-15 ) << i;
  }
}

// ----------------------------------------------------------------------------
class get_image : public ::testing::Test
{
  TEST_ARG( data_dir );
};

// ----------------------------------------------------------------------------
TEST_F ( get_image, crop )
{
  kwiver::arrows::gdal::image_io img_io;

  kwiver::vital::path_t file_path = data_dir + "/" + png_file_name;

  auto img_cont = img_io.load( file_path );

  test_get_image_crop< uint8_t >( img_cont );
}
