// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief core image class tests

#include <arrows/tests/test_image.h>

#include <vital/types/image.h>
#include <vital/types/image_container.h>
#include <vital/util/transform_image.h>
#include <vital/vital_config.h>

#include <gtest/gtest.h>

using namespace kwiver::vital;

// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

namespace // anonymous
{

// ----------------------------------------------------------------------------
template <unsigned int W, unsigned int H> byte
value_at( unsigned int i, unsigned int j, unsigned int k )
{
  constexpr static unsigned int is = 1;
  constexpr static unsigned int js = W;
  constexpr static unsigned int ks = W * H;

  return static_cast<byte>( ( ( ks * k ) + ( js * j ) + ( is * i ) ) % 255 );
}

// ----------------------------------------------------------------------------
void check_equal( image_memory mem_1, image_memory mem_2 )
{
  char* mem_1_data = (char*) mem_1.data();
  char* mem_2_data = (char*) mem_2.data();

  EXPECT_EQ( mem_1.size(), mem_2.size() );
  for(unsigned int i=0; mem_1_data[i]!='\0'; i++)
  {
    EXPECT_EQ( mem_1_data[i], mem_2_data[i] );
  }
}

// ----------------------------------------------------------------------------
void check_equal( image img_1, image img_2 )
{
  EXPECT_EQ( img_1.width(),   img_2.width()  );
  EXPECT_EQ( img_1.height(),  img_2.height() );
  EXPECT_EQ( img_1.depth(),   img_2.depth()  );
  EXPECT_EQ( img_1.w_step(),  img_2.w_step() );
  EXPECT_EQ( img_1.h_step(),  img_2.h_step() );
  EXPECT_EQ( img_1.d_step(),  img_2.d_step() );
  EXPECT_EQ( img_1.memory(),  img_2.memory() );
  EXPECT_EQ( img_1.first_pixel(),   img_2.first_pixel() );
  EXPECT_EQ( img_1.pixel_traits(),  img_2.pixel_traits() );
}

// Helper methods for tests in this file; for use in the transform_image
// function

// ----------------------------------------------------------------------------
struct val_zero_op
{
  byte operator()( byte const & //b
 )
  {
    return 0;
  }
};

// ----------------------------------------------------------------------------
struct val_incr_op
{
  byte operator()( VITAL_UNUSED byte const &b )
  {
    return val_incr_op_i++;
  }

  byte val_incr_op_i = 0;
};

}

// ----------------------------------------------------------------------------
TEST(image_pixel_traits, print)
{
  image_pixel_traits traits;
  std::stringstream output;

  traits.type = image_pixel_traits::UNKNOWN;
  traits.num_bytes = 1;
  output << traits;
  EXPECT_EQ( output.str(), "Unknown_1" );

  output.str(std::string());
  traits.type = image_pixel_traits::UNSIGNED;
  traits.num_bytes = 2;
  output << traits;
  EXPECT_EQ( output.str(), "Unsigned_2" );

  output.str(std::string());
  traits.type = image_pixel_traits::SIGNED;
  traits.num_bytes = 3;
  output << traits;
  EXPECT_EQ( output.str(), "Signed_3" );

  output.str(std::string());
  traits.type = image_pixel_traits::FLOAT;
  traits.num_bytes = 4;
  output << traits;
  EXPECT_EQ( output.str(), "Float_4" );

  output.str(std::string());
  traits.type = image_pixel_traits::BOOL;
  traits.num_bytes = 5;
  output << traits;
  EXPECT_EQ( output.str(), "Bool_5" );
}

// ----------------------------------------------------------------------------
TEST(image_memory, copy_constructor)
{
  std::string set_data = "test";
  image_memory mem{ set_data.size()*sizeof( char ) };

  char* data = (char*) mem.data();
  for(unsigned int i=0; i<set_data.size(); i++)
  {
    data[i] = set_data[i];
  }

  image_memory mem_copy{ mem };
  check_equal( mem, mem_copy );
}

// ----------------------------------------------------------------------------
TEST(image_memory, assignment_operator)
{
  std::string set_data = "test";
  image_memory mem{ set_data.size()*sizeof( char ) };

  char* data = (char*) mem.data();
  for(unsigned int i=0; i<set_data.size(); i++)
  {
    data[i] = set_data[i];
  }

  image_memory mem_assigned;
  mem_assigned = mem;
  check_equal( mem, mem_assigned );

  // copy of an image_memory from itself
  mem_assigned = mem_assigned;
  check_equal( mem, mem_assigned );
}

// ----------------------------------------------------------------------------
TEST(image, default_constructor)
{
  image img;
  EXPECT_EQ( 0, img.size() );
  EXPECT_EQ( nullptr, img.first_pixel() );
  EXPECT_EQ( 0, img.width() );
  EXPECT_EQ( 0, img.height() );
  EXPECT_EQ( 0, img.depth() );
  EXPECT_EQ( 1, img.pixel_traits().num_bytes );
  EXPECT_EQ( image_pixel_traits::UNSIGNED, img.pixel_traits().type );
}

// ----------------------------------------------------------------------------
TEST(image, constructor)
{
  image img_1{ 200, 300 };
  EXPECT_EQ( 200, img_1.width() );
  EXPECT_EQ( 300, img_1.height() );
  EXPECT_EQ( 1, img_1.depth() );
  EXPECT_EQ( 1, img_1.pixel_traits().num_bytes );
  EXPECT_EQ( 200 * 300 * 1, img_1.size() );

  image img_3{ 200, 300, 3 };
  EXPECT_EQ( 200, img_3.width() );
  EXPECT_EQ( 300, img_3.height() );
  EXPECT_EQ( 3, img_3.depth() );
  EXPECT_EQ( 1, img_3.pixel_traits().num_bytes );
  EXPECT_EQ( 200 * 300 * 3, img_3.size() );

  image_of<double> img_double{ 200, 300, 3 };
  EXPECT_EQ( 200, img_double.width() );
  EXPECT_EQ( 300, img_double.height() );
  EXPECT_EQ( 3, img_double.depth() );
  EXPECT_EQ( sizeof(double), img_double.pixel_traits().num_bytes );
  EXPECT_EQ( 200 * 300 * 3 * sizeof(double), img_double.size() );
}

// ----------------------------------------------------------------------------
TEST(image, copy_constructor)
{
  image_of<int> img{ 100, 75, 2 };
  image img_copy{ img };
  check_equal( img, img_copy );

  // copy an image_of from a base image
  image_of<int> img_copy_of_copy{ img_copy };
  check_equal( img, img_copy_of_copy );

  EXPECT_THROW( image_of<float> img_bad_copy{ img_copy },
                image_type_mismatch_exception );
}

// ----------------------------------------------------------------------------
TEST(image, assignment_operator)
{
  image_of<float> img{ 100, 75, 2 };
  image img_assigned;
  img_assigned = img;
  check_equal( img, img_assigned );

  // copy an image from itself
  img_assigned = img_assigned;
  check_equal( img, img_assigned );

  // copy an image_of from a base image
  image_of<float> img_assigned_again;
  img_assigned_again = img_assigned;
  check_equal( img, img_assigned_again );

  image_of<int> img_bad_assign;
  EXPECT_THROW( img_bad_assign = img_assigned,
                image_type_mismatch_exception );
}

// ----------------------------------------------------------------------------
TEST(image, equality_operator)
{
  image_of<float> img{ 100, 75, 2 };
  image img_assigned;
  img_assigned = img;
  EXPECT_EQ(img, img_assigned);
  // test inequality operator matches equality operator
  EXPECT_EQ(img != img_assigned, !(img == img_assigned));

  // copy an image_of from a base image
  image_of<float> img_assigned_again;
  img_assigned_again = img_assigned;
  EXPECT_EQ(img, img_assigned_again);

  // make a deep copy of the image
  // not considered equal by shallow comparison
  image_of<float> img_deep_copy;
  img_deep_copy.copy_from(img);
  EXPECT_NE(img, img_deep_copy);
}

// ----------------------------------------------------------------------------
TEST(image, set_size)
{
  image img{ 10, 20, 4 };
  void* data = img.first_pixel();
  img.set_size( 10, 20, 4 );
  EXPECT_EQ( data, img.first_pixel() )
    << "Calling set_size with the existing size should keep the same memory";

  // Keep another copy of the original image to prevent the original memory
  // from being deleted and then reallocated.
  image img_copy = img;
  img.set_size( 20, 10, 4 );
  EXPECT_NE( data, img.first_pixel() )
    << "Calling set_size with a new size should allocate new memory";
  EXPECT_EQ( 20, img.width() );
  EXPECT_EQ( 10, img.height() );
  EXPECT_EQ( 4, img.depth() );
}

// ----------------------------------------------------------------------------
TEST(image, is_contiguous)
{
  constexpr static unsigned w = 100, h = 200, d = 3;

  EXPECT_FALSE( image{}.is_contiguous() )
    << "Empty image should not be contiguous";

  EXPECT_TRUE( ( image{ w, h, d } ).is_contiguous() )
    << "Basic constructor should produce contiguous image";

  EXPECT_TRUE( ( image{ w, h, d, true } ).is_contiguous() )
    << "Interleaved image constructor should produce contiguous";

  // A view using every other row of a larger image
  auto mem = std::make_shared<image_memory>( w * h * d * 2 );
  EXPECT_FALSE(
    ( image{ mem, mem->data(), w, h, d, 1, 2 * w, 2 * w * h }.is_contiguous() ) )
    << "View of image skipping rows should be non-contiguous";

  EXPECT_FALSE(
    ( image{ mem, mem->data(), w, h, d, d, 2 * w * d, 1 }.is_contiguous() ) )
    << "View of interleaved image skipping rows should be non-contiguous";

  // An image with negative depth steps
  auto const first_byte = reinterpret_cast<byte*>( mem->data() ) + 2 * w * h;
  constexpr static auto d_step = -static_cast<ptrdiff_t>( w * h );
  EXPECT_FALSE(
    ( image{ mem, first_byte, w, h, d, 1, w, d_step }.is_contiguous() ) )
    << "Images with negative steps should be non-contiguous";
}

// ----------------------------------------------------------------------------
TEST(image, copy_from)
{
  constexpr static unsigned w = 100, h = 200, d = 3;
  image img1{ w, h, d };
  for ( unsigned k = 0; k < d; ++k )
  {
    for ( unsigned j = 0; j < h; ++j )
    {
      for ( unsigned i = 0; i < w; ++i )
      {
        img1.at<byte>( i, j, k ) = value_at<w, h>( i, j, k );
      }
    }
  }

  image img2{ image_pixel_traits{ image_pixel_traits::BOOL } };
  img2.copy_from( img1 );
  EXPECT_NE( img1.first_pixel(), img2.first_pixel() )
    << "Deep copied images should not share the same memory";
  EXPECT_TRUE( equal_content( img1, img2 ) );

  image_of<unsigned char> img3{ 200, 400, 3 };
  // create a view into the center of img3
  image_of<unsigned char> img4{
    img3.memory(), img3.first_pixel() + 50 * 200 + 50,
    w, h, d, 1, 200, 200 * 400 };

  // copy data into the view
  unsigned char* data = img4.first_pixel();
  img4.copy_from( img1 );
  EXPECT_EQ( data, img4.first_pixel() )
    << "Deep copying with the correct size should not reallocate memory";
  EXPECT_TRUE( equal_content( img1, img4 ) );
}

// ----------------------------------------------------------------------------
TEST(image, equal_content)
{
  constexpr static unsigned w = 100, h = 200, d = 3;
  image_of<byte> img1{ w, h, d };
  image_of<byte> img2{ w, h, d, true };
  EXPECT_NE( img1.memory(), img2.memory() );
  EXPECT_NE( img1.w_step(), img2.w_step() );

  for ( unsigned k = 0; k < d; ++k )
  {
    for ( unsigned j = 0; j < h; ++j )
    {
      for ( unsigned i = 0; i < w; ++i )
      {
        auto const v = value_at<w, h>( i, j, k );
        img1( i, j, k ) = v;
        img2( i, j, k ) = v;
      }
    }
  }
  EXPECT_TRUE( equal_content( img1, img2 ) );

  image img3( w, h, d, false, image_pixel_traits{ image_pixel_traits::BOOL } );
  EXPECT_FALSE( equal_content( img1, img3 ) );

  *img2.first_pixel()=1;
  EXPECT_FALSE( equal_content( img1, img2 ) );
}

// ----------------------------------------------------------------------------
// Testing that the transform image traverses pixels in memory order
class image_transform : public ::testing::Test
{
protected:
  constexpr static unsigned w = 3, h = 3, d = 3;
  using data_t = byte[w][h][d];

  void check_image( image_of<byte> const& img, data_t const& expected_data );
  void check_image_init( image_of<byte> const& img );
};

// ----------------------------------------------------------------------------
void
image_transform::
check_image( image_of<byte> const& img, data_t const& expected_data )
{
  for ( size_t i : { 0, 1, 2 } )
  {
    for ( size_t j : { 0, 1, 2 } )
    {
      for ( size_t k : { 0, 1, 2 } )
      {
        SCOPED_TRACE( "At " + std::to_string( i ) +
                      ", " + std::to_string( j ) +
                      ", " + std::to_string( k ) );
        EXPECT_EQ( expected_data[i][j][k], img( i, j, k ) );
      }
    }
  }
}

// ----------------------------------------------------------------------------
void image_transform::check_image_init( image_of<byte> const& img )
{
  constexpr static data_t zeroes = {
    { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
    { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } } };

  SCOPED_TRACE( "Zeroed image" );
  check_image( img, zeroes );
}

// ----------------------------------------------------------------------------
TEST_F(image_transform, basic)
{
  // An image with traditional stepping ( w < h < d )
  image_of<byte> img{ w, h, d, false };
  transform_image( img, val_zero_op{} );
  check_image_init( img );

  transform_image( img, val_incr_op{} );

  constexpr static data_t values = {
    { { 0,  9, 18 }, { 3, 12, 21 }, { 6, 15, 24 } },
    { { 1, 10, 19 }, { 4, 13, 22 }, { 7, 16, 25 } },
    { { 2, 11, 20 }, { 5, 14, 23 }, { 8, 17, 26 } } };

  SCOPED_TRACE( "Image after filling" );
  check_image( img, values );
}

// ----------------------------------------------------------------------------
TEST_F(image_transform, interleaved)
{
  // An interleaved image ( d < w < h )
  image_of<byte> img{ w, h, d, true };
  transform_image( img, val_zero_op{} );
  check_image_init( img );

  transform_image( img, val_incr_op{} );

  constexpr static data_t values = {
    { { 0, 1, 2 }, {  9, 10, 11 }, { 18, 19, 20 } },
    { { 3, 4, 5 }, { 12, 13, 14 }, { 21, 22, 23 } },
    { { 6, 7, 8 }, { 15, 16, 17 }, { 24, 25, 26 } } };

  SCOPED_TRACE( "Image after filling" );
  check_image( img, values );
}

// ----------------------------------------------------------------------------
TEST_F(image_transform, weird)
{
  // An image with a weird layout ( h < d < w )
  constexpr static ptrdiff_t hs = 1, ds = h, ws = d * h;
  image_memory mem{ w * h * d };
  image_of<byte> img{ reinterpret_cast<byte*>( mem.data() ),
                      w, h, d, ws, hs, ds };
  transform_image( img, val_zero_op{} );
  check_image_init( img );

  transform_image( img, val_incr_op{} );

  constexpr static data_t values = {
    { {  0,  3,  6 }, {  1,  4,  7 }, {  2,  5,  8 } },
    { {  9, 12, 15 }, { 10, 13, 16 }, { 11, 14, 17 } },
    { { 18, 21, 24 }, { 19, 22, 25 }, { 20, 23, 26 } } };

  SCOPED_TRACE( "Image after filling" );
  check_image( img, values );
}

// ----------------------------------------------------------------------------
TEST_F(image_transform, non_contiguous)
{
  // An image with a non-contiguous layout
  constexpr static ptrdiff_t ws = 7;
  constexpr static ptrdiff_t hs = w * ws + 11;
  constexpr static ptrdiff_t ds = h * hs * 3;
  image_memory mem{ d * ds };
  image_of<byte> img{ reinterpret_cast<byte*>( mem.data() ),
                      w, h, d, ws, hs, ds };
  transform_image( img, val_zero_op{} );
  check_image_init( img );

  transform_image( img, val_incr_op{} );

  constexpr static data_t values = {
    { { 0,  9, 18 }, { 3, 12, 21 }, { 6, 15, 24 } },
    { { 1, 10, 19 }, { 4, 13, 22 }, { 7, 16, 25 } },
    { { 2, 11, 20 }, { 5, 14, 23 }, { 8, 17, 26 } } };

  SCOPED_TRACE( "Image after filling" );
  check_image( img, values );
}

// ----------------------------------------------------------------------------
TEST(image, cast_image_of)
{
  image_of<uint16_t> img1{ 50, 50, 3 };
  image_of<bool> img2;

  cast_image( img1, img2 );

  EXPECT_EQ( img1.width(),  img2.width()  );
  EXPECT_EQ( img1.height(), img2.height() );
  EXPECT_EQ( img1.depth(),  img2.depth()  );
  EXPECT_EQ( img1.w_step(), img2.w_step() );
  EXPECT_EQ( img1.h_step(), img2.h_step() );
  EXPECT_EQ( img1.d_step(), img2.d_step() );
}

// ----------------------------------------------------------------------------
TEST(image, cast_image)
{
  image_of<uint16_t> img_in{ 50, 50, 3 };
  image img1 = img_in;
  image_of<bool> img2;
  cast_image( img1, img2 );

  EXPECT_EQ( img1.width(),  img2.width()  );
  EXPECT_EQ( img1.height(), img2.height() );
  EXPECT_EQ( img1.depth(),  img2.depth()  );
  EXPECT_EQ( img1.w_step(), img2.w_step() );
  EXPECT_EQ( img1.h_step(), img2.h_step() );
  EXPECT_EQ( img1.d_step(), img2.d_step() );
}

// ----------------------------------------------------------------------------
template <typename T, int Depth>
struct image_type
{
  using pixel_type = T;
  static constexpr int depth = Depth;
};

template <typename T>
class get_image : public ::testing::Test
{
};

using get_image_types = ::testing::Types<
  image_type<byte, 1>,
  image_type<byte, 3>,
  image_type<uint16_t, 1>,
  image_type<uint16_t, 3>,
  image_type<float, 1>,
  image_type<float, 3>,
  image_type<double, 1>,
  image_type<double, 3>
  >;

TYPED_TEST_CASE(get_image, get_image_types);

// ----------------------------------------------------------------------------
TYPED_TEST(get_image, crop)
{
  using pix_t = typename TypeParam::pixel_type;
  image_of<pix_t> img{ full_width, full_height, 3 };
  populate_vital_image<pix_t>( img );

  image_container_sptr img_cont = std::make_shared<simple_image_container>(img);

  test_get_image_crop<pix_t>( img_cont );
}
