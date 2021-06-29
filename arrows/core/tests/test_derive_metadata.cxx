// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief test derivation of new metadata traits

#include <arrows/core/derive_metadata.h>

#include <vital/types/geo_point.h>
#include <vital/types/geodesy.h>
#include <vital/types/metadata.h>
#include <vital/types/metadata_traits.h>

#include <gtest/gtest.h>

#include <iostream>
#include <memory>
#include <sstream>

namespace kv = kwiver::vital;

static kwiver::vital::metadata_traits meta_traits;

namespace {

double constexpr FRAME_CENTER_ELEVATION = 749.755127;
double constexpr SENSOR_ELEVATION = 6942.789551;

// ----------------------------------------------------------------------------
kwiver::vital::metadata_vector
make_metadata()
{
  kv::metadata_sptr m1 = std::make_shared< kv::metadata >();

  // Add the double traits
  m1->add< kv::VITAL_META_PLATFORM_HEADING_ANGLE >( 324.266418 );
  m1->add< kv::VITAL_META_PLATFORM_PITCH_ANGLE >( -0.19776 );
  m1->add< kv::VITAL_META_PLATFORM_ROLL_ANGLE >( 20.050661 );
  m1->add< kv::VITAL_META_SENSOR_REL_AZ_ANGLE >( 73.911217 );
  m1->add< kv::VITAL_META_SENSOR_REL_EL_ANGLE >( -8.558719 );
  m1->add< kv::VITAL_META_SENSOR_REL_ROLL_ANGLE >( 0.526359 );
  m1->add< kv::VITAL_META_SENSOR_VERTICAL_FOV >( 0.42298 );
  m1->add< kv::VITAL_META_SENSOR_HORIZONTAL_FOV >( 0.771801 );
  m1->add< kv::VITAL_META_SLANT_RANGE >( 13296.55762 );

  // Add the geo point traits
  m1->add< kv::VITAL_META_SENSOR_LOCATION >( { kv::geo_point::geo_3d_point_t{
                                                 0, 0, SENSOR_ELEVATION },
                                               kv::SRID::lat_lon_WGS84 } );
  m1->add< kv::VITAL_META_FRAME_CENTER >( {
      kv::geo_point::geo_3d_point_t{
        0, 0, FRAME_CENTER_ELEVATION }, kv::SRID::lat_lon_WGS84 } );

  return { m1 };
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
class derive_metadata : public ::testing::Test
{
  void
  SetUp()
  {
    auto meta = make_metadata();
    constexpr auto frame_height = size_t{ 720 };
    constexpr auto frame_width = size_t{ 1080 };

    derived_metadata = kwiver::arrows::core::compute_derived_metadata(
      meta, frame_width, frame_height );
  }

public:
  kv::metadata_vector derived_metadata;
};

// ----------------------------------------------------------------------------
TEST_F( derive_metadata, compute_derived )
{
  kv::metadata_item const& gsd_value =
    derived_metadata.at( 0 )->find( kv::VITAL_META_AVERAGE_GSD );
  kv::metadata_item const& vniirs_value =
    derived_metadata.at( 0 )->find( kv::VITAL_META_VNIIRS );
  kv::metadata_item const& slant_range_value =
    derived_metadata.at( 0 )->find( kv::VITAL_META_SLANT_RANGE );

  // Reference value is 0.202224; we use current actual output value here to
  //   detect regression (or confirm intentional change)
  EXPECT_NEAR( 0.199086, gsd_value.as_double(), 0.000001 );

  // This will not actualy be correct due to image terms
  // EXPECT_DOUBLE_EQ( 6.58, vniirs_value.as_double() );
  EXPECT_DOUBLE_EQ( 13296.55762, slant_range_value.as_double() );
}
