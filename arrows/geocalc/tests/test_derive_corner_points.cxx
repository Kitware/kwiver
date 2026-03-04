// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test the derive_corner_points metadata filter

#include <test_gtest.h>

#include <arrows/geocalc/algo/derive_corner_points.h>

#include <vital/math_constants.h>
#include <vital/plugin_management/plugin_manager.h>
#include <vital/types/geodesy.h>

#include <limits>

using namespace kwiver;
namespace geocalc = arrows::geocalc;

// ----------------------------------------------------------------------------
vital::metadata_sptr
create_base_metadata()
{
  auto metadata = std::make_shared< vital::metadata >();
  metadata->add< vital::VITAL_META_SENSOR_ORIENTATION >(
    vital::rotation_d{
    45.57747768368048 * vital::deg_to_rad,
    3.69518112735374 * vital::deg_to_rad,
    11.125522629474777 * vital::deg_to_rad, } *
    vital::rotation_d{
    94.4019541177903 * vital::deg_to_rad,
    -35.31228668769462 * vital::deg_to_rad,
    0.08038890549922104 * vital::deg_to_rad, } );
  metadata->add< vital::VITAL_META_SENSOR_LOCATION >(
    vital::geo_point{
    vital::vector_3d{
      138.49543155100916, -34.77187584376516, 5173.090714885176 },
    vital::SRID::lat_lon_WGS84 } );
  metadata->add< vital::VITAL_META_SENSOR_HORIZONTAL_FOV >(
    0.4559395742732891 );
  metadata->add< vital::VITAL_META_SENSOR_VERTICAL_FOV >(
    0.2581826504921035 );
  metadata->add< vital::VITAL_META_FRAME_CENTER >(
    vital::geo_point{
    vital::vector_3d{
      138.53192434577827, -34.80403506886402, 5.497825589379659 },
    vital::SRID::lat_lon_WGS84 } );
  return metadata;
}

// ----------------------------------------------------------------------------
int
main( int argc, char* argv[] )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( derive_corner_points, create )
{
  EXPECT_NE(
    nullptr,
    vital::create_algorithm< vital::algo::metadata_filter >(
      "derive_corner_points" ) );
}

// ----------------------------------------------------------------------------
TEST ( derive_corner_points, filter_simple )
{
  geocalc::derive_corner_points filter;

  auto metadata = create_base_metadata();

  std::vector< vital::vector_2d > expected_corners = {
    vital::vector_2d{ 138.53232947899156, -34.804032779975806 },
    vital::vector_2d{ 138.53182134580879, -34.804332624331408 },
    vital::vector_2d{ 138.53151692367675, -34.804032779975806 },
    vital::vector_2d{ 138.53202047908312, -34.803730646731992 } };

  auto const results = filter.filter( { metadata }, nullptr );
  ASSERT_EQ( 1, results.size() );
  ASSERT_NE( nullptr, results[ 0 ] );
  ASSERT_TRUE( results[ 0 ]->has( vital::VITAL_META_CORNER_POINTS ) );

  auto const filtered_corners =
    results[ 0 ]->find( vital::VITAL_META_CORNER_POINTS )
    .get< vital::geo_polygon >()
    .polygon( vital::SRID::lat_lon_WGS84 )
    .get_vertices();
  ASSERT_EQ( 4, filtered_corners.size() );

  for( size_t i = 0; i < 4; ++i )
  {
    SCOPED_TRACE( i );
    for( size_t j = 0; j < 2; ++j )
    {
      SCOPED_TRACE( j );
      EXPECT_NEAR(
        expected_corners[ i ][ j ], filtered_corners[ i ][ j ],
        2.0e-5 );
    }
  }
}

// ----------------------------------------------------------------------------
TEST ( derive_corner_points, altitude_only_with_nan_latlon )
{
  geocalc::derive_corner_points filter;

  auto metadata = create_base_metadata();
  metadata->add< vital::VITAL_META_FRAME_CENTER >(
    vital::geo_point{
    vital::vector_3d{
      std::numeric_limits< double >::quiet_NaN(),
      std::numeric_limits< double >::quiet_NaN(),
      5.497825589379659 },
    vital::SRID::lat_lon_WGS84 } );

  std::vector< vital::vector_2d > expected_corners = {
    vital::vector_2d{ 138.53236499433206, -34.804064572349347 },
    vital::vector_2d{ 138.53187025547868, -34.804375687947555 },
    vital::vector_2d{ 138.53156096073721, -34.804070633053733 },
    vital::vector_2d{ 138.53205342688352, -34.803760671453098 } };

  auto const results = filter.filter( { metadata }, nullptr );
  ASSERT_EQ( 1, results.size() );
  ASSERT_NE( nullptr, results[ 0 ] );
  ASSERT_TRUE( results[ 0 ]->has( vital::VITAL_META_CORNER_POINTS ) );

  auto const filtered_corners =
    results[ 0 ]->find( vital::VITAL_META_CORNER_POINTS )
    .get< vital::geo_polygon >()
    .polygon( vital::SRID::lat_lon_WGS84 )
    .get_vertices();
  ASSERT_EQ( 4, filtered_corners.size() );

  for( size_t i = 0; i < 4; ++i )
  {
    SCOPED_TRACE( i );
    for( size_t j = 0; j < 2; ++j )
    {
      SCOPED_TRACE( j );
      EXPECT_NEAR(
        expected_corners[ i ][ j ], filtered_corners[ i ][ j ],
        2.0e-5 );
    }
  }
}

// ----------------------------------------------------------------------------
TEST ( derive_corner_points, missing_frame_center_uses_zero_elevation )
{
  geocalc::derive_corner_points filter;

  auto metadata = create_base_metadata();
  metadata->erase( vital::VITAL_META_FRAME_CENTER );

  std::vector< vital::vector_2d > expected_corners = {
    vital::vector_2d{ 138.53236499433206, -34.804064572349347 },
    vital::vector_2d{ 138.53187025547868, -34.804375687947555 },
    vital::vector_2d{ 138.53156096073721, -34.804070633053733 },
    vital::vector_2d{ 138.53205342688352, -34.803760671453098 } };

  auto const results = filter.filter( { metadata }, nullptr );
  ASSERT_EQ( 1, results.size() );
  ASSERT_NE( nullptr, results[ 0 ] );
  ASSERT_TRUE( results[ 0 ]->has( vital::VITAL_META_CORNER_POINTS ) );

  auto const filtered_corners =
    results[ 0 ]->find( vital::VITAL_META_CORNER_POINTS )
    .get< vital::geo_polygon >()
    .polygon( vital::SRID::lat_lon_WGS84 )
    .get_vertices();
  ASSERT_EQ( 4, filtered_corners.size() );

  for( size_t i = 0; i < 4; ++i )
  {
    SCOPED_TRACE( i );
    for( size_t j = 0; j < 2; ++j )
    {
      SCOPED_TRACE( j );
      EXPECT_NEAR(
        expected_corners[ i ][ j ], filtered_corners[ i ][ j ],
        2.0e-5 );
    }
  }
}
