// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include <test_gtest.h>

#include <arrows/geocalc/constants.h>
#include <arrows/geocalc/geo_conv.h>

#include <vital/plugin_management/plugin_manager.h>

#include <sstream>

using namespace kwiver;
using namespace vital::SRID;

namespace {

// ----------------------------------------------------------------------------
// Precision within a millimeter should be sufficient
constexpr double epsilon_meters = 1.0e-3;
constexpr double epsilon_degrees = 1.0e-8;

} // namespace

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  TEST_LOAD_PLUGINS();

  return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
TEST ( geo_conv, via_vital )
{
  using E = arrows::geocalc::ellipsoid_wgs84;

  arrows::geocalc::geo_conversion converter;
  vital::set_geo_conv( &converter );

  vital::vector_3d input{ 0.0, 0.0, 0.0 };
  vital::vector_3d output = vital::geo_conv( input, lat_lon_WGS84, ECEF_WGS84 );

  EXPECT_NEAR( E::a, output[ 0 ], epsilon_meters );
  EXPECT_NEAR( 0.0, output[ 1 ], epsilon_meters );
  EXPECT_NEAR( 0.0, output[ 2 ], epsilon_meters );
}

// ----------------------------------------------------------------------------
TEST ( geo_conv, wgs84_geodetic_ecef )
{
  using E = arrows::geocalc::ellipsoid_wgs84;

  arrows::geocalc::geo_conversion converter;

  std::vector< std::pair< vital::vector_3d, vital::vector_3d > > pairs = {
    // (Geodetic, ecef) values from (uses proj underneath):
    // https://tool-online.com/en/coordinate-converter.php

    // Regular locations
    { { -73.7737921, 42.8644703, 500 },
      { 1308453.486687, -4496049.222089, 4316818.360666 } },
    { { -73.7737921, -42.8644703, 500 },
      { 1308453.486687, -4496049.222089, -4316818.360666 } },
    { { 180.0, 42.8644703, 500 }, { -4682575.053778787, 0.0, 4316818.360666 } },

    // Near equator
    { { 0.0, 0.0, 0.0 }, { E::a, 0.0, 0.0 } },
    { { 0.0, 0.000001, -1000.0 }, { 6377137.0, 0.0, 0.110557 } },
    { { 42.0, -0.000001, 1000.0 },
      { 4740622.652561, 4268475.808856, -0.110557 } },

    // Near poles
    { { 0.0, 90.0, 0.0 }, { 0.0, 0.0, E::b } },
    { { 0.0, 90.0, 100.0 }, { 0.0, 0.0, E::b + 100.0 } },
    { { 0.0, 89.999'999, 0.0 }, { 0.111694, 0.0, 6356752.314245 } },
    { { 90.0, 89.999'999, 100.0 }, { 0.0, 0.111696, 6356852.314245 } },
    { { 0.0, -90.0, -100.0 }, { 0.0, 0.0, -E::b + 100.0 } },
    { { 0.0, -89.999'999, 0.0 }, { 0.111694, 0.0, -6356752.314245 } },
    { { -90.0, -89.999'999, 100.0 }, { 0.0, -0.111696, -6356852.314245 } },

    // Extreme altitudes
    { { -73.7737921, 42.8644703, 1'000'000 },
      { 1513163.697734, -5199465.273553, 4996744.708175 } },
    { { -73.7737921, 42.8644703, -1'000'000 },
      { 1103538.463023, -3791929.402690, 3636211.746677 } },
    { { 45.0, 46.0, 600'000'000 },
      { 297856952.860, 297856952.860, 436169127.744 } },
    { { 0.0, 0.0, -E::a }, { 0.0, 0.0, 0.0 } },
    { { 0.0, 0.0, -E::a + 1.0 }, { 1.0, 0.0, 0.0 } },
    { { 63.43494882, 89.99700970, -6356749.314 }, { 1.0, 2.0, 3.0 } }, };

  size_t i = 0;
  for( auto const& [ geodetic, ecef ] : pairs )
  {
    SCOPED_TRACE( i );
    ++i;

    auto const converted_geodetic =
      converter( ecef, ECEF_WGS84, lat_lon_WGS84 );
    EXPECT_NEAR( geodetic[ 0 ], converted_geodetic[ 0 ], epsilon_degrees );
    EXPECT_NEAR( geodetic[ 1 ], converted_geodetic[ 1 ], epsilon_degrees );
    EXPECT_NEAR( geodetic[ 2 ], converted_geodetic[ 2 ], epsilon_meters );

    auto const converted_ecef =
      converter( geodetic, lat_lon_WGS84, ECEF_WGS84 );
    EXPECT_NEAR( ecef[ 0 ], converted_ecef[ 0 ], epsilon_meters );
    EXPECT_NEAR( ecef[ 1 ], converted_ecef[ 1 ], epsilon_meters );
    EXPECT_NEAR( ecef[ 2 ], converted_ecef[ 2 ], epsilon_meters );
  }
}

// ----------------------------------------------------------------------------
TEST ( geo_conv, nonfinite )
{
  arrows::geocalc::geo_conversion converter;

  auto const nan = std::numeric_limits< double >::quiet_NaN();
  auto const inf = std::numeric_limits< double >::infinity();
  for( auto const& point : {
    vital::vector_3d{ nan, 0.0, 0.0 },
    vital::vector_3d{ inf, 0.0, 0.0 },
    vital::vector_3d{ 0.0, nan, 0.0 },
    vital::vector_3d{ 0.0, inf, 0.0 },
    vital::vector_3d{ 0.0, 0.0, nan },
    vital::vector_3d{ 0.0, 0.0, inf },
    vital::vector_3d{ nan, nan, nan },
    vital::vector_3d{ inf, inf, inf }, } )
  {
    for( auto const& [ crs1, crs2 ] : {
      std::make_pair( ECEF_WGS84, lat_lon_WGS84 ),
      std::make_pair( lat_lon_WGS84, ECEF_WGS84 ), } )
    {
      auto const converted = converter( point, crs1, crs2 );
      EXPECT_TRUE( std::isnan( converted[ 0 ] ) );
      EXPECT_TRUE( std::isnan( converted[ 1 ] ) );
      EXPECT_TRUE( std::isnan( converted[ 2 ] ) );
    }
  }
}
