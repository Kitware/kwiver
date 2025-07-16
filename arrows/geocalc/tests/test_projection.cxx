// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_gtest.h>

#include <arrows/geocalc/constants.h>
#include <arrows/geocalc/geo_conv.h>
#include <arrows/geocalc/projection.h>

#include <vital/math_constants.h>
#include <vital/types/geodesy.h>

#include <cmath>

using namespace kwiver;
using namespace vital::SRID;

namespace {

// ----------------------------------------------------------------------------
// Precision within a centimeter should be sufficient
constexpr double epsilon_meters = 1.0e-2;

// ----------------------------------------------------------------------------
void
test_raycast_wgs84(
  std::optional< vital::vector_3d > const& expected,
  vital::vector_3d const& point,
  std::optional< vital::vector_3d > const& vector,
  double altitude )
{
  if( !expected && !vector )
  {
    throw std::logic_error( "Invalid test values" );
  }

  auto const v = vector ? *vector : ( *expected - point ).normalized();
  auto const result = arrows::geocalc::raycast_ecef_to_ellipsoid(
    point, v, ECEF_WGS84, altitude );
  if( expected )
  {
    ASSERT_TRUE( result );
    EXPECT_NEAR( ( *expected )[ 0 ], ( *result )[ 0 ], epsilon_meters );
    EXPECT_NEAR( ( *expected )[ 1 ], ( *result )[ 1 ], epsilon_meters );
    EXPECT_NEAR( ( *expected )[ 2 ], ( *result )[ 2 ], epsilon_meters );
  }
  else
  {
    EXPECT_FALSE( result );
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
// Projecting straight down
TEST ( projection, raycast_wgs84_down )
{
  using E = arrows::geocalc::ellipsoid_wgs84;

  arrows::geocalc::geo_conversion converter;

  CALL_TEST(
    test_raycast_wgs84,
    vital::vector_3d( E::a, 0.0, 0.0 ),
    vital::vector_3d( E::a + 10'000.0, 0.0, 0.0 ),
    std::nullopt,
    0.0 );

  CALL_TEST(
    test_raycast_wgs84,
    vital::vector_3d( E::a + 0.5, 0.0, 0.0 ),
    vital::vector_3d( E::a + 1.0, 0.0, 0.0 ),
    std::nullopt,
    0.5 );

  CALL_TEST(
    test_raycast_wgs84,
    vital::vector_3d( 0.0, 0.0, E::b + 100.0 ),
    vital::vector_3d( 0.0, 0.0, E::b + 10'000.0 ),
    std::nullopt,
    100.0 );

  CALL_TEST(
    test_raycast_wgs84,
    vital::vector_3d( 0.0, 0.0, -E::b + 1'000.0 ),
    vital::vector_3d( 0.0, 0.0, -E::b + 500.0 ),
    std::nullopt,
    -1'000.0 );

  CALL_TEST(
    test_raycast_wgs84,
    converter(
      vital::vector_3d( 30.0, 60.0, 500.0 ), lat_lon_WGS84, ECEF_WGS84 ),
    converter(
      vital::vector_3d( 30.0, 60.0, 1'000.0 ), lat_lon_WGS84, ECEF_WGS84 ),
    std::nullopt,
    500.0 );

  CALL_TEST(
    test_raycast_wgs84,
    converter(
      vital::vector_3d( -30.0, -60.0, -500.0 ), lat_lon_WGS84, ECEF_WGS84 ),
    converter(
      vital::vector_3d( -30.0, -60.0, 10'000.0 ), lat_lon_WGS84, ECEF_WGS84 ),
    std::nullopt,
    -500.0 );
}

// ----------------------------------------------------------------------------
// Projecting toward the Earth
TEST ( projection, raycast_wgs84_downish )
{
  arrows::geocalc::geo_conversion converter;

  CALL_TEST(
    test_raycast_wgs84,
    converter(
      vital::vector_3d( 30.0, 60.0, 500.0 ), lat_lon_WGS84, ECEF_WGS84 ),
    converter(
      vital::vector_3d( 30.01, 60.01, 1'000.0 ), lat_lon_WGS84, ECEF_WGS84 ),
    std::nullopt,
    500.0 );

  CALL_TEST(
    test_raycast_wgs84,
    converter(
      vital::vector_3d( -30.0, -60.0, -500.0 ), lat_lon_WGS84, ECEF_WGS84 ),
    converter(
      vital::vector_3d( -30.01, -60.01, 10'000.0 ), lat_lon_WGS84, ECEF_WGS84 ),
    std::nullopt,
    -500.0 );

  CALL_TEST(
    test_raycast_wgs84,
    converter(
      vital::vector_3d( 0.0, 0.0, 0.0 ), lat_lon_WGS84, ECEF_WGS84 ),
    converter(
      vital::vector_3d( 0.001, -0.001, 10'000.0 ), lat_lon_WGS84, ECEF_WGS84 ),
    std::nullopt,
    0.0 );

  CALL_TEST(
    test_raycast_wgs84,
    converter(
      vital::vector_3d( 0.0, 89.999999, 20.0 ), lat_lon_WGS84, ECEF_WGS84 ),
    converter(
      vital::vector_3d( 0.0, 90.0, 100.0 ), lat_lon_WGS84, ECEF_WGS84 ),
    std::nullopt,
    20.0 );
}

// ----------------------------------------------------------------------------
// Missing the Earth
TEST ( projection, raycast_wgs84_misses )
{
  using E = arrows::geocalc::ellipsoid_wgs84;

  CALL_TEST(
    test_raycast_wgs84,
    std::nullopt,
    vital::vector_3d( E::a + 10.0, 0.0, 0.0 ),
    vital::vector_3d( 1.0, 0.0, 0.0 ),
    0.0 );

  CALL_TEST(
    test_raycast_wgs84,
    std::nullopt,
    vital::vector_3d( E::a + 10'000.0, 0.0, 0.0 ),
    vital::vector_3d( 0.0, 1.0, 0.0 ),
    0.0 );

  CALL_TEST(
    test_raycast_wgs84,
    std::nullopt,
    vital::vector_3d( E::a + 10'000.0, 0.0, 0.0 ),
    vital::vector_3d( 0.0, 0.0, 1.0 ),
    5'000.0 );

  CALL_TEST(
    test_raycast_wgs84,
    std::nullopt,
    vital::vector_3d( E::a - 100.0, 0.0, 0.0 ),
    vital::vector_3d( 0.0, 0.0, 1.0 ),
    -500.0 );

  CALL_TEST(
    test_raycast_wgs84,
    std::nullopt,
    vital::vector_3d( 0.0, 0.0, -E::b - 1'000.0 ),
    vital::vector_3d( 1.0, 0.0, 0.0 ),
    0.0 );

  CALL_TEST(
    test_raycast_wgs84,
    std::nullopt,
    vital::vector_3d( 0.0, 0.0, -E::b - 1'000.0 ),
    vital::vector_3d(
      std::cos( 1.0 * vital::deg_to_rad ),
      0.0,
      std::sin( 1.0 * vital::deg_to_rad ) ),
    0.0 );
}

// ----------------------------------------------------------------------------
// Ray starts inside ellipsoid
TEST ( projection, raycast_wgs84_interior )
{
  using E = arrows::geocalc::ellipsoid_wgs84;
  EXPECT_THROW(
    arrows::geocalc::raycast_ecef_to_ellipsoid(
      vital::vector_3d( 0.0, E::a, 0.0 ),
      vital::vector_3d( 0.0, -1.0, 0.0 ), ECEF_WGS84, 10.0 );
    ,
    std::runtime_error );

  EXPECT_NO_THROW(
    arrows::geocalc::raycast_ecef_to_ellipsoid(
      vital::vector_3d( 0.0, E::a, 0.0 ),
      vital::vector_3d( 0.0, -1.0, 0.0 ), ECEF_WGS84, -10.0 ) );

  std::optional< vital::vector_3d > result;
  EXPECT_NO_THROW(
    result = arrows::geocalc::raycast_ecef_to_ellipsoid(
      vital::vector_3d( 0.0, E::a, 0.0 ),
      vital::vector_3d( 0.0, -1.0, 0.0 ), ECEF_WGS84, 10.0, false ) );

  ASSERT_TRUE( result );
  EXPECT_NEAR( 0.0, ( *result )[ 0 ], epsilon_meters );
  EXPECT_NEAR( -E::a - 10.0, ( *result )[ 1 ], epsilon_meters );
  EXPECT_NEAR( 0.0, ( *result )[ 2 ], epsilon_meters );
}

// ----------------------------------------------------------------------------
TEST ( projection, raycast_nonfinite )
{
  using E = arrows::geocalc::ellipsoid_wgs84;

  auto const nan = std::numeric_limits< double >::quiet_NaN();
  auto const inf = std::numeric_limits< double >::infinity();

  EXPECT_THROW(
    arrows::geocalc::raycast_ecef_to_ellipsoid(
      vital::vector_3d( E::a + 10.0, 0.0, 0.0 ),
      vital::vector_3d( -1.0, 0.0, 0.0 ), ECEF_WGS84, inf );
    ,
    std::runtime_error );

  EXPECT_THROW(
    arrows::geocalc::raycast_ecef_to_ellipsoid(
      vital::vector_3d( E::a + 10.0, 0.0, 0.0 ),
      vital::vector_3d( -1.0, 0.0, 0.0 ), ECEF_WGS84, nan );
    ,
    std::runtime_error );

  EXPECT_THROW(
    arrows::geocalc::raycast_ecef_to_ellipsoid(
      vital::vector_3d( E::a + 10.0, 0.0, 0.0 ),
      vital::vector_3d( -inf, 0.0, 0.0 ), ECEF_WGS84, 0.0 );
    ,
    std::runtime_error );

  EXPECT_THROW(
    arrows::geocalc::raycast_ecef_to_ellipsoid(
      vital::vector_3d( E::a + 10.0, 0.0, 0.0 ),
      vital::vector_3d( -nan, 0.0, 0.0 ), ECEF_WGS84, 0.0 );
    ,
    std::runtime_error );

  EXPECT_THROW(
    arrows::geocalc::raycast_ecef_to_ellipsoid(
      vital::vector_3d( E::a + 10.0, 0.0, inf ),
      vital::vector_3d( -1.0, 0.0, 0.0 ), ECEF_WGS84, 0.0 );
    ,
    std::runtime_error );

  EXPECT_THROW(
    arrows::geocalc::raycast_ecef_to_ellipsoid(
      vital::vector_3d( E::a + 10.0, 0.0, nan ),
      vital::vector_3d( -1.0, 0.0, 0.0 ), ECEF_WGS84, 0.0 );
    ,
    std::runtime_error );
}
