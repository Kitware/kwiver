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

// ----------------------------------------------------------------------------
int
main( int argc, char** argv )
{
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

namespace {

// ----------------------------------------------------------------------------
kwiver::vital::metadata_vector
make_metadata()
{
  kv::metadata_sptr m1 = std::make_shared< kv::metadata >();

  // Add the double-valued traits
  std::map< kwiver::vital::vital_metadata_tag, double >
  double_metadata{ { kv::VITAL_META_PLATFORM_HEADING_ANGLE, 60 },
    { kv::VITAL_META_PLATFORM_PITCH_ANGLE, 3 },
    { kv::VITAL_META_PLATFORM_ROLL_ANGLE, 1.2 },
    { kv::VITAL_META_SENSOR_REL_AZ_ANGLE, 5 },
    { kv::VITAL_META_SENSOR_REL_EL_ANGLE, 4 },
    { kv::VITAL_META_SENSOR_REL_ROLL_ANGLE, 5 },
    { kv::VITAL_META_PLATFORM_ROLL_ANGLE, 0.5 },
    { kv::VITAL_META_SENSOR_VERTICAL_FOV, 30 },
    { kv::VITAL_META_SENSOR_HORIZONTAL_FOV, 40 },
    { kv::VITAL_META_SLANT_RANGE, 300 },
    { kv::VITAL_META_TARGET_WIDTH, 100 } };

  for( auto const& elem : double_metadata )
  {
    auto const& trait = meta_traits.find( elem.first );
    m1->add( trait.create_metadata_item( elem.second ) );
  }

  // Add the geo point traits
  m1->add< kv::VITAL_META_SENSOR_LOCATION >(
    { kv::geo_point::geo_3d_point_t{ 0, 0, 12 },
      kv::SRID::lat_lon_WGS84 } );

  auto const& frame_center_trait =
    meta_traits.find( kv::VITAL_META_FRAME_CENTER );
  m1->add( frame_center_trait.create_metadata_item(
             kv::geo_point( kv::geo_point::geo_3d_point_t{ 0, 0, 0 },
                            kv::SRID::lat_lon_WGS84 ) ) );

  // Replicate these values except without the slant range field
  auto const m2 = std::make_shared< kv::metadata >( m1->deep_copy() );
  m2->erase( kv::VITAL_META_SLANT_RANGE );
  return { m1, m2 };
}

} // namespace <anonymous>

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
TEST_F(derive_metadata, compute_derived)
{
  kv::metadata_item const& gsd_value =
    derived_metadata.at( 0 )->find( kv::VITAL_META_AVERAGE_GSD );
  kv::metadata_item const& vniirs_value =
    derived_metadata.at( 0 )->find( kv::VITAL_META_VNIIRS );
  kv::metadata_item const& slant_range_value =
    derived_metadata.at( 0 )->find( kv::VITAL_META_SLANT_RANGE );

  EXPECT_DOUBLE_EQ( 0.38253304, gsd_value.as_double() );
  EXPECT_DOUBLE_EQ( 5.183558, vniirs_value.as_double() );
  EXPECT_DOUBLE_EQ( 300, slant_range_value.as_double() );
}

// ----------------------------------------------------------------------------
TEST_F(derive_metadata, compute_derived_no_slant)
{
  // Use the estimated slant range
  kv::metadata_item const& gsd_value = derived_metadata.at( 1 )->find(
    kv::VITAL_META_AVERAGE_GSD );
  kv::metadata_item const& vniirs_value = derived_metadata.at( 1 )->find(
    kv::VITAL_META_VNIIRS );
  kv::metadata_item const& slant_range_value = derived_metadata.at( 1 )->find(
    kv::VITAL_META_SLANT_RANGE );

  EXPECT_FLOAT_EQ( gsd_value.as_double(), 0.43601143 );
  EXPECT_FLOAT_EQ( vniirs_value.as_double(), 4.9948859 );
  EXPECT_FLOAT_EQ( slant_range_value.as_double(), 341.94022 );
}
