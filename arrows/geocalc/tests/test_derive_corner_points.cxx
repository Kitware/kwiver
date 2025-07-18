// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Test the derive_corner_points metadata filter

#include <test_gtest.h>

#include <arrows/geocalc/algo/derive_corner_points.h>

#include <vital/plugin_management/plugin_manager.h>
#include <vital/types/geodesy.h>

using namespace kwiver;
namespace geocalc = arrows::geocalc;

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

  // Values taken from frame 4 of public WASABI video:
  // 20151007_064904_dstoMX20HD__EON_High_013.mpg
  auto metadata = std::make_shared< vital::metadata >();
  metadata->add< vital::VITAL_META_PLATFORM_HEADING_ANGLE >(
    45.57747768368048 );
  metadata->add< vital::VITAL_META_PLATFORM_PITCH_ANGLE >(
    3.69518112735374 );
  metadata->add< vital::VITAL_META_PLATFORM_ROLL_ANGLE >(
    11.125522629474777 );
  metadata->add< vital::VITAL_META_SENSOR_LOCATION >(
    vital::geo_point{
    vital::vector_3d{
      138.49543155100916, -34.77187584376516, 5173.090714885176 },
    vital::SRID::lat_lon_WGS84 } );
  metadata->add< vital::VITAL_META_SENSOR_HORIZONTAL_FOV >(
    0.4559395742732891 );
  metadata->add< vital::VITAL_META_SENSOR_VERTICAL_FOV >(
    0.2581826504921035 );
  metadata->add< vital::VITAL_META_SENSOR_REL_AZ_ANGLE >(
    94.40195411779033 );
  metadata->add< vital::VITAL_META_SENSOR_REL_EL_ANGLE >(
    -35.31228668769462 );
  metadata->add< vital::VITAL_META_SENSOR_REL_ROLL_ANGLE >(
    0.08038890549922104 );
  metadata->add< vital::VITAL_META_FRAME_CENTER >(
    vital::geo_point{
    vital::vector_3d{
      138.53192434577827, -34.80403506886402, 5.497825589379659 },
    vital::SRID::lat_lon_WGS84 } );

  std::vector< vital::vector_2d > corners = {
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
      EXPECT_NEAR( corners[ i ][ j ], filtered_corners[ i ][ j ], 2.0e-5 );
    }
  }
}
