// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test visit_image utility functions.

#include <vital/util/visit_image.txx>

#include <tests/test_gtest.h>

using namespace kwiver::vital;

namespace {

// ----------------------------------------------------------------------------
template < class T1, class T2, class Visitor >
void
expect_scalar_visitor_called(
  Visitor&& visitor, image const& img1,
  image const& img2 )
{
  for( size_t i = 0; i < img1.width(); ++i )
  {
    for( size_t j = 0; j < img1.height(); ++j )
    {
      for( size_t k = 0; k < img1.depth(); ++k )
      {
        auto value1 = img1.at< T1 >( i, j, k );
        T2 value2;
        visitor( value1, value2 );
        ASSERT_EQ( value2, img2.at< T2 >( i, j, k ) );
      }
    }
  }
}

// ----------------------------------------------------------------------------
template < class T, size_t... Indices >
auto
_tie_pixel_at(
  image const& img, size_t i, size_t j,
  // UNCRUST-OFF
  std::integer_sequence< size_t, Indices... >
  // UNCRUST-ON
)
{
  // UNCRUST-OFF
  return std::tie( img.at< T >( i, j, Indices )... );
  // UNCRUST-ON
}

// ----------------------------------------------------------------------------
template < class T, size_t N >
auto
tie_pixel_at( image const& img, size_t i, size_t j )
{
  return _tie_pixel_at< T >( img, i, j, std::make_index_sequence< N >{} );
}

// ----------------------------------------------------------------------------
template < size_t Depth1, size_t Depth2, class T1, class T2, class Visitor >
void
expect_pixel_visitor_called(
  Visitor&& visitor, image const& img1,
  image const& img2 )
{
  for( size_t i = 0; i < img1.width(); ++i )
  {
    for( size_t j = 0; j < img1.height(); ++j )
    {
      auto value1 = tie_pixel_at< T1, Depth1 >( img1, i, j );
      auto expected_value2 = tie_pixel_at< T2, Depth2 >( img2, i, j );
      sized_tuple< T2, Depth2 > value2_vals;
      auto value2 = tie_tuple( value2_vals );
      visitor( value1, value2 );
      ASSERT_EQ( value2_vals, expected_value2 );
    }
  }
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
class visit_image : public ::testing::Test
{
public:
  void
  SetUp() override
  {
    img_planar = image{ 15, 18, 3, false, image_pixel_traits_of< uint8_t >() };
    img_packed = image{ 15, 18, 3, true, image_pixel_traits_of< uint8_t >() };
    img_sparse =
      image{
      img_packed.memory(), img_packed.first_pixel(),
      5, 3, 2, 3 * 3, 6 * 15 * 3, 2, img_packed.pixel_traits() };
    img_neg =
      image{
      img_packed.memory(),
      static_cast< uint8_t* >( img_packed.first_pixel() ) + 15 * 18 * 3 - 3,
      15, 18, 3,
      -img_packed.w_step(), -img_packed.h_step(), img_packed.d_step(),
      img_packed.pixel_traits() };
    img_float = image{ 15, 18, 3, true, image_pixel_traits_of< float >() };

    for( size_t i = 0; i < 15 * 18 * 3; ++i )
    {
      static_cast< uint8_t* >( img_planar.first_pixel() )[ i ] = i % 137;
      static_cast< uint8_t* >( img_packed.first_pixel() )[ i ] = i % 137;
      static_cast< float* >( img_float.first_pixel() )[ i ] =
        static_cast< float >( i % 137 );
    }
  }

  image img_planar;
  image img_packed;
  image img_sparse;
  image img_neg;
  image img_float;
};

// ----------------------------------------------------------------------------
TEST_F ( visit_image, scalars_const )
{
  size_t sum;
  auto const visitor = [ &sum ]( uint8_t x ){ sum += x; };

  for( auto const& [ img, expected_sum ] : {
    std::make_pair( img_planar, 54330 ),
    std::make_pair( img_packed, 54330 ),
    std::make_pair( img_sparse, 998 ),
    std::make_pair( img_neg, 54330 ), } )
  {
    sum = 0;
    visit_image_scalars< void >( visitor, image{ img } );
    EXPECT_EQ( expected_sum, sum );

    sum = 0;
    visit_image_scalars< uint8_t const >( visitor, img );
    EXPECT_EQ( expected_sum, sum );
  }
}

// ----------------------------------------------------------------------------
TEST_F ( visit_image, scalars_mutate )
{
  auto const visitor = []( auto& x ){ x = 255; };
  for( auto img_ptr : { &img_planar, &img_packed } )
  {
    auto& img = *img_ptr;
    visit_image_scalars( visitor, img );
    for( size_t i = 0; i < img.width(); ++i )
    {
      for( size_t j = 0; j < img.height(); ++j )
      {
        for( size_t k = 0; k < img.depth(); ++k )
        {
          ASSERT_EQ( 255, img.at< uint8_t >( i, j, k ) );
        }
      }
    }
  }

  {
    auto& img = img_float;
    visit_image_scalars( visitor, img );
    for( size_t i = 0; i < img.width(); ++i )
    {
      for( size_t j = 0; j < img.height(); ++j )
      {
        for( size_t k = 0; k < img.depth(); ++k )
        {
          ASSERT_EQ( 255.0f, img.at< float >( i, j, k ) );
        }
      }
    }
  }
}

// ----------------------------------------------------------------------------
TEST_F ( visit_image, scalars2_same_type )
{
  size_t sum;
  auto const visitor =
    [ &sum ]( uint8_t a, uint8_t b ){
      sum += a > b ? a - b : b - a;
    };

  for( auto const& [ img1, img2, expected_sum ] : {
    std::make_tuple( img_planar, img_packed, 34008 ),
    std::make_tuple( img_planar, img_neg, 38468 ), } )
  {
    sum = 0;
    visit_image_scalars2( visitor, img1, img2 );
    EXPECT_EQ( expected_sum, sum );

    sum = 0;
    visit_image_scalars2< uint8_t >( visitor, img1, img2 );
    EXPECT_EQ( expected_sum, sum );

    sum = 0;
    visit_image_scalars2< uint8_t, uint8_t >( visitor, img1, img2 );
    EXPECT_EQ( expected_sum, sum );

    sum = 0;
    visit_image_scalars2< void, void, false >( visitor, img1, img2 );
    EXPECT_EQ( expected_sum, sum );
  }
}

// ----------------------------------------------------------------------------
TEST_F ( visit_image, scalars2_different_type )
{
  auto const visitor =
    []( uint8_t a, auto& b ){
      b = static_cast< std::decay_t< decltype( b ) > >( a ^ 0b10100101 );
    };
  for( auto const& img1 : { img_packed, img_planar, img_neg } )
  {
    {
      auto img2 = img_float.copy( false );
      visit_image_scalars2< void, void, false >( visitor, img1, img2 );
      CALL_TEST(
        ( expect_scalar_visitor_called< uint8_t, float > ),
        visitor, img1, img2 );
    }

    {
      auto img2 = img_float.copy( true );
      visit_image_scalars2< uint8_t, float >( visitor, img1, img2 );
      CALL_TEST(
        ( expect_scalar_visitor_called< uint8_t, float > ),
        visitor, img1, img2 );
    }

    {
      auto img2 = img_float.copy( false );
      visit_image_scalars2< uint8_t, void, false >( visitor, img1, img2 );
      CALL_TEST(
        ( expect_scalar_visitor_called< uint8_t, float > ),
        visitor, img1, img2 );
    }

    {
      auto img2 = img_float.copy( true );
      visit_image_scalars2< void, float, false >( visitor, img1, img2 );
      CALL_TEST(
        ( expect_scalar_visitor_called< uint8_t, float > ),
        visitor, img1, img2 );
    }
  }
}

// ----------------------------------------------------------------------------
TEST_F ( visit_image, scalars2_create )
{
  auto const visitor =
    []( uint8_t a, auto& b ){
      b = static_cast< std::decay_t< decltype( b ) > >( a + 1 );
    };
  for( auto const& img1 : { img_packed, img_planar, img_neg, img_sparse } )
  {
    {
      auto const img2 = visit_image_scalars2_create( visitor, img1 );
      CALL_TEST(
        ( expect_scalar_visitor_called< uint8_t, uint8_t > ),
        visitor, img1, img2 );
    }

    {
      auto const img2 =
        visit_image_scalars2_create< void, float >( visitor, img1 );
      CALL_TEST(
        ( expect_scalar_visitor_called< uint8_t, float > ),
        visitor, img1, img2 );
    }

    {
      auto const img2 =
        visit_image_scalars2_create< uint8_t, uint16_t >( visitor, img1 );
      CALL_TEST(
        ( expect_scalar_visitor_called< uint8_t, uint16_t > ),
        visitor, img1, img2 );
    }
  }
}

// ----------------------------------------------------------------------------
TEST_F ( visit_image, pixels )
{
  size_t sum;
  auto const visitor =
    [ &sum ]( sized_tuple< uint8_t const&, 3 > rgb ){
      auto& [ r, g, b ] = rgb;
      sum += std::min( r, b );
    };

  for( auto const& img : { img_packed, img_planar, img_neg } )
  {
    {
      sum = 0;
      visit_image_pixels< 3 >( visitor, img );
      EXPECT_EQ( 17574, sum );
    }

    {
      sum = 0;
      visit_image_pixels< 3, uint8_t >( visitor, img );
      EXPECT_EQ( 17574, sum );
    }
  }
}

// ----------------------------------------------------------------------------
TEST_F ( visit_image, pixels2 )
{
  auto const visitor =
    []( auto rgb, auto bgr ){
      auto& [ r0, g0, b0 ] = rgb;
      auto& [ r1, g1, b1 ] = bgr;
      r1 = r0;
      g1 = g0;
      b1 = b0;
    };

  for( auto const& img1 : { img_packed, img_planar, img_neg } )
  {
    {
      auto img2 = img_float.copy( false );
      visit_image_pixels2< 3, 3, void, void, false >( visitor, img1, img2 );
      CALL_TEST(
        ( expect_pixel_visitor_called< 3, 3, uint8_t, float > ),
        visitor, img1, img2 );
    }

    {
      auto img2 = img_float.copy( true );
      visit_image_pixels2< 3, 3, uint8_t, float >( visitor, img1, img2 );
      CALL_TEST(
        ( expect_pixel_visitor_called< 3, 3, uint8_t, float > ),
        visitor, img1, img2 );
    }

    {
      auto img2 = img_float.copy( false );
      visit_image_pixels2< 3, 3, uint8_t, void, false >( visitor, img1, img2 );
      CALL_TEST(
        ( expect_pixel_visitor_called< 3, 3, uint8_t, float > ),
        visitor, img1, img2 );
    }

    {
      auto img2 = img_float.copy( true );
      visit_image_pixels2< 3, 3, void, float, false >( visitor, img1, img2 );
      CALL_TEST(
        ( expect_pixel_visitor_called< 3, 3, uint8_t, float > ),
        visitor, img1, img2 );
    }
  }
}

// ----------------------------------------------------------------------------
TEST_F ( visit_image, pixels2_create )
{
  for( auto const& img1 : { img_packed, img_planar, img_neg } )
  {
    auto const visitor =
      []( auto p0, auto p1 ){
        auto& [ r0, g0, b0 ] = p0;
        auto& [ r1, g1, b1, a1 ] = p1;
        r1 = r0;
        g1 = g0;
        b1 = b0;
        a1 = 1;
      };

    {
      auto const img2 = visit_image_pixels2_create< 3, 4 >( visitor, img1 );
      CALL_TEST(
        ( expect_pixel_visitor_called< 3, 4, uint8_t, uint8_t > ),
        visitor, img1, img2 );
    }

    {
      auto const img2 =
        visit_image_pixels2_create< 3, 4, void, float >( visitor, img1 );
      CALL_TEST(
        ( expect_pixel_visitor_called< 3, 4, uint8_t, float > ),
        visitor, img1, img2 );
    }

    {
      auto const img2 =
        visit_image_pixels2_create< 3, 4, uint8_t, uint16_t >( visitor, img1 );
      CALL_TEST(
        ( expect_pixel_visitor_called< 3, 4, uint8_t, uint16_t > ),
        visitor, img1, img2 );
    }
  }

  {
    auto const visitor =
      []( auto p0, auto p1 ){
        auto& [ r0, g0 ] = p0;
        auto& [ r1, g1, b1, a1 ] = p1;
        r1 = r0;
        g1 = g0;
        b1 = 1;
        a1 = 1;
      };

    auto const& img1 = img_sparse;

    {
      auto const img2 = visit_image_pixels2_create< 2, 4 >( visitor, img1 );
      CALL_TEST(
        ( expect_pixel_visitor_called< 2, 4, uint8_t, uint8_t > ),
        visitor, img1, img2 );
    }

    {
      auto const img2 =
        visit_image_pixels2_create< 2, 4, void, float >( visitor, img1 );
      CALL_TEST(
        ( expect_pixel_visitor_called< 2, 4, uint8_t, float > ),
        visitor, img1, img2 );
    }

    {
      auto const img2 =
        visit_image_pixels2_create< 2, 4, uint8_t, uint16_t >( visitor, img1 );
      CALL_TEST(
        ( expect_pixel_visitor_called< 2, 4, uint8_t, uint16_t > ),
        visitor, img1, img2 );
    }
  }
}
