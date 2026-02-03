// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <tests/test_gtest.h>

#include <arrows/geocalc/geo_conv.h>

#include <vital/math_constants.h>
#include <vital/plugin_management/plugin_manager.h>
#include <vital/types/geodesy.h>
#include <vital/types/local_tangent_space.h>

#include <Eigen/Geometry>

#include <cmath>

using namespace kwiver;
using namespace kwiver::vital;

namespace {

// ----------------------------------------------------------------------------
// Precision within a millimeter should be sufficient
constexpr double epsilon_meters = 1.0e-3;
constexpr double epsilon_degrees = 1.0e-8;
constexpr double epsilon_quaternion = 1.0e-7;

// ----------------------------------------------------------------------------
std::vector< vital::rotation_d > const rotations = {
  {},
  { vital::pi / 2, 0.0, 0.0 },
  { 0.0, vital::pi / 3, 0.0 },
  { 0.0, 0.0, vital::pi / 4 },
  { -vital::pi / 2, -vital::pi / 3, -vital::pi / 4 },
  { vital::pi, vital::pi, vital::pi }, };

// ----------------------------------------------------------------------------
void
expect_rotation_near(
  vital::rotation_d const& a, vital::rotation_d const& b )
{
  auto const q_a = a.quaternion();
  auto const q_b = b.quaternion();
  auto const w =
    std::clamp( std::abs( ( q_a * q_b.conjugate() ).w() ), 0.0, 1.0 );
  auto const angle = 2.0 * std::acos( w );
  EXPECT_LT( angle, epsilon_quaternion );
}

// ----------------------------------------------------------------------------
// Test that the coordinate system is right-handed
void
test_right_handed( local_tangent_space const& space )
{
  auto const point1 =
    space.to_global( { 0.0, 0.0, 0.0 } ).location( SRID::ECEF_WGS84 );
  auto const point2 =
    space.to_global( { 1.0, 0.0, 0.0 } ).location( SRID::ECEF_WGS84 );
  auto const point3 =
    space.to_global( { 0.0, 1.0, 0.0 } ).location( SRID::ECEF_WGS84 );
  auto const point4 =
    space.to_global( { 0.0, 0.0, 1.0 } ).location( SRID::ECEF_WGS84 );

  vector_3d const v1 = ( point2 - point1 ).cross( point3 - point1 );
  vector_3d const v2 = ( point4 - point1 );
  EXPECT_NEAR( v1[ 0 ], v2[ 0 ], epsilon_meters );
  EXPECT_NEAR( v1[ 1 ], v2[ 1 ], epsilon_meters );
  EXPECT_NEAR( v1[ 2 ], v2[ 2 ], epsilon_meters );
}

// ----------------------------------------------------------------------------
// Test that the coordinate system uses meters in a cartesian space
void
test_cartesian_meters( local_tangent_space const& space )
{
  auto const point1 =
    space.to_global( { 0.0, 0.0, 0.0 } ).location( SRID::ECEF_WGS84 );
  auto const point2 =
    space.to_global( { 3'000.0, 4'000.0, 5'000.0 } )
    .location( SRID::ECEF_WGS84 );

  EXPECT_NEAR(
    std::sqrt( 50'000'000.0 ), ( point2 - point1 ).norm(), epsilon_meters );
}

// ----------------------------------------------------------------------------
// Test that the X axis is east and Y axis is north
void
test_east_north( local_tangent_space const& space )
{
  auto const point1 =
    space.to_global( { 0.0, 0.0, 300.0 } ).location( SRID::lat_lon_WGS84 );
  auto const point2 =
    space.to_global( { -1'000.0, 0.0, 300.0 } ).location( SRID::lat_lon_WGS84 );
  auto const point3 =
    space.to_global( { 0.0, -2'000.0, 300.0 } ).location( SRID::lat_lon_WGS84 );

  // X axis - latitude and height should not change much
  EXPECT_NEAR( point1[ 1 ], point2[ 1 ], 1.0e-6 );
  EXPECT_NEAR( point1[ 2 ], point2[ 2 ], 0.5 );

  // Y axis - longitude should not change, height should not change much
  EXPECT_NEAR( point1[ 0 ], point3[ 0 ], epsilon_degrees );
  EXPECT_NEAR( point1[ 2 ], point3[ 2 ], 1.0 );
}

// ----------------------------------------------------------------------------
// Test that the Z axis is up
void
test_up( local_tangent_space const& space )
{
  auto const point1 =
    space.to_global( { 0.0, 0.0, 300.0 } ).location( SRID::lat_lon_WGS84 );
  auto const point2 =
    space.to_global( { 0.0, 0.0, 100'300.0 } ).location( SRID::lat_lon_WGS84 );

  // Z axis - latitude and longitude should not change
  EXPECT_NEAR( point1[ 0 ], point2[ 0 ], epsilon_degrees );
  EXPECT_NEAR( point1[ 1 ], point2[ 1 ], epsilon_degrees );
  EXPECT_NEAR( 100'000.0, point2[ 2 ] - point1[ 2 ], epsilon_meters );
}

// ----------------------------------------------------------------------------
// Test that global -> local -> global yields the same original points
void
test_global_round_trip( local_tangent_space const& space )
{
  std::vector< geo_point > const points = {
    { vector_3d{ 0.0, 0.0, 0.0 }, SRID::ECEF_WGS84 },
    { vector_3d{ 0.0, 90.0, 100.0 }, SRID::lat_lon_WGS84 },
    { vector_3d{ 42.0, 89.999'999, -10.0 }, SRID::lat_lon_WGS84 },
    { vector_3d{ -42.0, -89.999'999, 0.0 }, SRID::lat_lon_WGS84 },
    { vector_3d{ 0.0, 0.0, 0.0 }, SRID::lat_lon_WGS84 },
    { vector_3d{ 90.0, 0.0, 30.0 }, SRID::lat_lon_WGS84 },
    { vector_3d{ 180.0, 0.0, -1'000.0 }, SRID::lat_lon_WGS84 },
    { vector_3d{ -90.0, 0.0, 100'000.0 }, SRID::lat_lon_WGS84 }, };

  for( size_t i = 0; i < points.size(); ++i )
  {
    SCOPED_TRACE( i );

    auto const& point = points[ i ];
    auto const result =
      space.to_global( space.to_local( point ) ).location( SRID::ECEF_WGS84 );
    auto const original = point.location( SRID::ECEF_WGS84 );
    EXPECT_NEAR( original[ 0 ], result[ 0 ], epsilon_meters );
    EXPECT_NEAR( original[ 1 ], result[ 1 ], epsilon_meters );
    EXPECT_NEAR( original[ 2 ], result[ 2 ], epsilon_meters );

    for( size_t j = 0; j < rotations.size(); ++j )
    {
      SCOPED_TRACE( j );

      auto const& rotation = rotations[ j ];
      auto const rotation_result =
        space.to_local( space.to_global( rotation, point ), point );
      CALL_TEST( expect_rotation_near, rotation_result, rotation );
    }
  }
}

// ----------------------------------------------------------------------------
// Test that local -> global -> local yields the same original points
void
test_local_round_trip( local_tangent_space const& space )
{
  std::vector< vector_3d > const points = {
    { 0.0, 0.0, 0.0 },
    { 1.0, 2.0, 3.0 },
    { -3.0, -2.0, -1.0 },
    { 1.0e-6, 2.0e-6, 3.0e-6 },
    { 3.0e6, 2.0e6, 1.0e6 }, };

  for( size_t i = 0; i < points.size(); ++i )
  {
    SCOPED_TRACE( i );

    auto const& point = points[ i ];
    auto const result = space.to_local( space.to_global( point ) );
    EXPECT_NEAR( point[ 0 ], result[ 0 ], epsilon_meters );
    EXPECT_NEAR( point[ 1 ], result[ 1 ], epsilon_meters );
    EXPECT_NEAR( point[ 2 ], result[ 2 ], epsilon_meters );

    for( size_t j = 0; j < rotations.size(); ++j )
    {
      SCOPED_TRACE( j );

      auto const& rotation = rotations[ j ];
      auto const global_point = space.to_global( point );
      auto const rotation_result =
        space.to_local(
          space.to_global( rotation, global_point ),
          global_point );
      CALL_TEST( expect_rotation_near, rotation_result, rotation );
    }
  }
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );

  arrows::geocalc::geo_conversion converter;
  set_geo_conv( &converter );

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( local_tangent_space, origin_invalid )
{
  vector_3d const local_point{ 0, 0, 0 };
  geo_point const global_point{ vector_3d{ 0, 0, 0 }, SRID::ECEF_WGS84 };
  vital::rotation_d rotation;
  for( auto const& space : {
    local_tangent_space{},
    local_tangent_space{ geo_point{} } } )
  {
    EXPECT_FALSE( space.valid() );
    EXPECT_TRUE( space.origin().is_empty() );
    EXPECT_THROW(
      space.to_local( global_point );
      ,
      std::runtime_error );
    EXPECT_THROW(
      space.to_local( rotation, global_point );
      ,
      std::runtime_error );
    EXPECT_THROW(
      space.to_global( local_point );
      ,
      std::runtime_error );
    EXPECT_THROW(
      space.to_global( rotation, global_point );
      ,
      std::runtime_error );
  }
}

// ----------------------------------------------------------------------------
TEST ( local_tangent_space, origin_at_center_of_earth )
{
  local_tangent_space space{
    { vector_3d{ 0.0, 0.0, -6'378'137.0 }, SRID::lat_lon_WGS84 } };
  EXPECT_TRUE( space.valid() );
  CALL_TEST( test_right_handed, space );
  CALL_TEST( test_cartesian_meters, space );
  CALL_TEST( test_global_round_trip, space );
  CALL_TEST( test_local_round_trip, space );
}

// ----------------------------------------------------------------------------
TEST ( local_tangent_space, origin_at_north_pole )
{
  local_tangent_space space{
    { vector_3d{ 0.0, 90.0, 0.0 }, SRID::lat_lon_WGS84 } };
  EXPECT_TRUE( space.valid() );
  CALL_TEST( test_right_handed, space );
  CALL_TEST( test_cartesian_meters, space );
  CALL_TEST( test_global_round_trip, space );
  CALL_TEST( test_local_round_trip, space );
  CALL_TEST( test_up, space );
}

// ----------------------------------------------------------------------------
TEST ( local_tangent_space, origin_near_north_pole )
{
  local_tangent_space space{
    { vector_3d{ 90.0, 89.999'999'999, -500.0 }, SRID::lat_lon_WGS84 } };
  EXPECT_TRUE( space.valid() );
  CALL_TEST( test_right_handed, space );
  CALL_TEST( test_cartesian_meters, space );
  CALL_TEST( test_global_round_trip, space );
  CALL_TEST( test_local_round_trip, space );
  CALL_TEST( test_up, space );
}

// ----------------------------------------------------------------------------
TEST ( local_tangent_space, origin_at_south_pole )
{
  local_tangent_space space{
    { vector_3d{ 180.0, -90.0, 0.0 }, SRID::lat_lon_WGS84 } };
  EXPECT_TRUE( space.valid() );
  CALL_TEST( test_right_handed, space );
  CALL_TEST( test_cartesian_meters, space );
  CALL_TEST( test_global_round_trip, space );
  CALL_TEST( test_local_round_trip, space );
  CALL_TEST( test_up, space );
}

// ----------------------------------------------------------------------------
TEST ( local_tangent_space, origin_near_south_pole )
{
  local_tangent_space space{
    { vector_3d{ -90.0, -89.999'999'999, 500.0 }, SRID::lat_lon_WGS84 } };
  EXPECT_TRUE( space.valid() );
  CALL_TEST( test_right_handed, space );
  CALL_TEST( test_cartesian_meters, space );
  CALL_TEST( test_global_round_trip, space );
  CALL_TEST( test_local_round_trip, space );
  CALL_TEST( test_up, space );
}

// ----------------------------------------------------------------------------
TEST ( local_tangent_space, origin_at_equator )
{
  local_tangent_space space{
    { vector_3d{ 179.0, 0.0, 0.0 }, SRID::lat_lon_WGS84 } };
  EXPECT_TRUE( space.valid() );
  CALL_TEST( test_right_handed, space );
  CALL_TEST( test_cartesian_meters, space );
  CALL_TEST( test_global_round_trip, space );
  CALL_TEST( test_local_round_trip, space );
  CALL_TEST( test_east_north, space );
  CALL_TEST( test_up, space );
}

// ----------------------------------------------------------------------------
TEST ( local_tangent_space, origin_near_equator )
{
  local_tangent_space space{
    { vector_3d{ -179.0, 0.000'001, 10.0 }, SRID::lat_lon_WGS84 } };
  EXPECT_TRUE( space.valid() );
  CALL_TEST( test_right_handed, space );
  CALL_TEST( test_cartesian_meters, space );
  CALL_TEST( test_global_round_trip, space );
  CALL_TEST( test_local_round_trip, space );
  CALL_TEST( test_east_north, space );
  CALL_TEST( test_up, space );
}

// ----------------------------------------------------------------------------
TEST ( local_tangent_space, arbitrary_origin_northern_hemisphere )
{
  local_tangent_space space{
    { vector_3d{ -73.7737921, 42.8644703, 50'000.0 }, SRID::lat_lon_WGS84 } };
  EXPECT_TRUE( space.valid() );
  CALL_TEST( test_right_handed, space );
  CALL_TEST( test_cartesian_meters, space );
  CALL_TEST( test_global_round_trip, space );
  CALL_TEST( test_local_round_trip, space );
  CALL_TEST( test_east_north, space );
  CALL_TEST( test_up, space );
}

// ----------------------------------------------------------------------------
TEST ( local_tangent_space, arbitrary_origin_southern_hemisphere )
{
  local_tangent_space space{
    { vector_3d{ 73.7737921, -42.8644703, -500.0 }, SRID::lat_lon_WGS84 } };
  EXPECT_TRUE( space.valid() );
  CALL_TEST( test_right_handed, space );
  CALL_TEST( test_cartesian_meters, space );
  CALL_TEST( test_global_round_trip, space );
  CALL_TEST( test_local_round_trip, space );
  CALL_TEST( test_east_north, space );
  CALL_TEST( test_up, space );
}

// ----------------------------------------------------------------------------
TEST ( local_tangent_space, rotations_enu_geodetic )
{
  local_tangent_space space{
    { vector_3d{ 30.0, 0.0, 100.0 }, SRID::lat_lon_WGS84 } };

  std::vector<
    std::tuple< rotation_d, vector_3d, rotation_d > > geodetic_cases = {
    { { 0.0, 0.0, 0.0 },
      { 30.0, 0.0, 0.0 },
      { 0.0, 0.0, 0.0 } },
    { { vital::pi / 2.0, vital::pi / 3.0, vital::pi / 4.0 },
      { 30.0, 0.0, 10.0 },
      { vital::pi / 2.0, vital::pi / 3.0, vital::pi / 4.0 } },
    { { 0.0, 0.0, 0.0 },
      { 120.0, 0.0, 0.0 },
      { 0.0, vital::pi / 2.0, 0.0 } },
    { { vital::pi / 3.0, 0.0, 0.0 },
      {  120.0, 0.0, 0.0 },
      { vital::pi / 3.0, vital::pi / 2.0, 0.0 } },
    { { 0.0, 0.0, 0.0 },
      { 30.0, 45.0, 0.0 },
      { 0.0, 0.0, -vital::pi / 4.0 } } };

  for( size_t i = 0; i < geodetic_cases.size(); ++i )
  {
    SCOPED_TRACE( i );
    auto const& [ rotation, point, expected ] = geodetic_cases[ i ];

    auto const result =
      space.to_global( rotation, geo_point{ point, SRID::lat_lon_WGS84 } );
    CALL_TEST( expect_rotation_near, expected, result );
  }
}

// ----------------------------------------------------------------------------
TEST ( local_tangent_space, rotations_enu_geocentric )
{
  local_tangent_space space{
    { vector_3d{ 90.0, 45.0, 100.0 }, SRID::lat_lon_WGS84 } };

  std::vector<
    std::tuple< rotation_d, vector_3d, rotation_d > > geocentric_cases = {
    { { 0.0, 0.0, 0.0 },
      { 90.0, 0.0, 0.0 },
      { vital::pi, 0.0, vital::pi / 4.0 } },
    { { 0.0, 0.0, vital::pi / 2.0 },
      { 0.0, 80.0, 2.0 },
      { vital::pi, 0.0, -vital::pi / 4.0 } }, };

  for( size_t i = 0; i < geocentric_cases.size(); ++i )
  {
    SCOPED_TRACE( i );
    auto const& [ rotation, point, expected ] = geocentric_cases[ i ];

    auto const result =
      space.to_global( rotation, geo_point{ point, SRID::ECEF_WGS84 } );
    CALL_TEST( expect_rotation_near, expected, result );
  }
}
