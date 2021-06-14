// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Compute derived metadata fields.

#include <arrows/core/derive_metadata.h>

#include <vital/types/geo_point.h>
#include <vital/types/metadata.h>
#include <vital/types/metadata_tags.h>
#include <vital/types/metadata_traits.h>
#include <vital/types/rotation.h>

#include <vital/math_constants.h>

#include <memory>

#include <cmath>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace core {

namespace {

static ::kwiver::vital::metadata_traits meta_traits;

// ----------------------------------------------------------------------------
kwiver::vital::rotation_d
get_platform_rotation( kwiver::vital::metadata_sptr const& metadata )
{
  kv::metadata_item const& yaw_item =
    metadata->find( kv::VITAL_META_PLATFORM_HEADING_ANGLE );
  kv::metadata_item const& pitch_item =
    metadata->find( kv::VITAL_META_PLATFORM_PITCH_ANGLE );
  kv::metadata_item const& roll_item =
    metadata->find( kv::VITAL_META_PLATFORM_ROLL_ANGLE );

  if( !yaw_item || !pitch_item || !roll_item )
  {
    VITAL_THROW( kv::invalid_value,
                 "metadata does not contain platform orientation" );
  }

  auto const yaw = yaw_item.as_double() * kv::deg_to_rad;
  auto const pitch = pitch_item.as_double() * kv::deg_to_rad;
  auto const roll = roll_item.as_double() * kv::deg_to_rad;

  return { yaw, pitch, roll };
}

// ----------------------------------------------------------------------------
kwiver::vital::rotation_d
get_sensor_rotation( kwiver::vital::metadata_sptr const& metadata )
{
  // All relative to platform
  kv::metadata_item const& yaw_item =
    metadata->find( kv::VITAL_META_SENSOR_REL_AZ_ANGLE );
  kv::metadata_item const& pitch_item =
    metadata->find( kv::VITAL_META_SENSOR_REL_EL_ANGLE );
  kv::metadata_item const& roll_item =
    metadata->find( kv::VITAL_META_SENSOR_REL_ROLL_ANGLE );

  if( !yaw_item || !pitch_item || !roll_item )
  {
    VITAL_THROW( kv::invalid_value,
                 "metadata does not contain sensor orientation" );
  }

  auto const yaw = yaw_item.as_double() * kv::deg_to_rad;
  auto const pitch = pitch_item.as_double() * kv::deg_to_rad;
  auto const roll = roll_item.as_double() * kv::deg_to_rad;

  return { yaw, pitch, roll };
}

// ----------------------------------------------------------------------------
kwiver::vital::rotation_d
get_total_rotation( kwiver::vital::metadata_sptr const& metadata )
{
  // Absolute (not relative to platform)
  kv::rotation_d const platform_rotation = get_platform_rotation( metadata );
  kv::rotation_d const sensor_rotation = get_sensor_rotation( metadata );
  return platform_rotation * sensor_rotation;
}

// ----------------------------------------------------------------------------
kwiver::vital::vector_2d
get_sensor_fov( kwiver::vital::metadata_sptr metadata )
{
  kv::metadata_item const& x_fov_item =
    metadata->find( kv::VITAL_META_SENSOR_HORIZONTAL_FOV );
  kv::metadata_item const& y_fov_item =
    metadata->find( kv::VITAL_META_SENSOR_VERTICAL_FOV );

  if( !x_fov_item || !y_fov_item )
  {
    VITAL_THROW( kv::invalid_value,
                 "metadata does not contain sensor fov" );
  }

  auto const x_fov = x_fov_item.as_double() * kv::deg_to_rad;
  auto const y_fov = y_fov_item.as_double() * kv::deg_to_rad;

  return { x_fov, y_fov };
}

// ----------------------------------------------------------------------------
double
get_slant_range( kwiver::vital::metadata_sptr const& metadata )
{
  kv::metadata_item const& item =
    metadata->find( kv::VITAL_META_SLANT_RANGE );

  if( !item )
  {
    VITAL_THROW( kv::invalid_value,
                 "metadata does not contain slant range" );
  }

  return item.as_double();
}

// ----------------------------------------------------------------------------
kv::geo_point
get_sensor_location( kwiver::vital::metadata_sptr const& metadata )
{
  kv::metadata_item const& item =
    metadata->find( kv::VITAL_META_SENSOR_LOCATION );

  if( !item )
  {
    VITAL_THROW( kv::invalid_value,
                 "metadata does not contain sensor location" );
  }

  return kv::any_cast< kv::geo_point >( item.data() );
}

// ----------------------------------------------------------------------------
kv::geo_point
get_frame_center( kwiver::vital::metadata_sptr const& metadata )
{
  kv::metadata_item const& item =
    metadata->find( kv::VITAL_META_FRAME_CENTER );

  if( !item )
  {
    VITAL_THROW( kv::invalid_value,
                 "metadata does not contain frame center" );
  }

  return kv::any_cast< kv::geo_point >( item.data() );
}

// ----------------------------------------------------------------------------
double
get_target_width( kwiver::vital::metadata_sptr const& metadata )
{
  kv::metadata_item const& item =
    metadata->find( kv::VITAL_META_TARGET_WIDTH );

  if( !item )
  {
    VITAL_THROW( kv::invalid_value,
                 "metadata does not contain target width" );
  }

  return item.as_double();
}

// ----------------------------------------------------------------------------
double
compute_slant_range( kwiver::vital::metadata_sptr const& metadata )
{
  double slant_range = 0.0;
  try
  {
    // Attempt to acquire slant range directly
    slant_range = get_slant_range( metadata );
  }
  catch ( kv::invalid_value const& e )
  {
    // Slant range must be calculated from other values
    kv::rotation_d const total_rotation = get_total_rotation( metadata );
    double yaw, pitch, roll;
    total_rotation.get_yaw_pitch_roll( yaw, pitch, roll );

    // Determine the altitude of the sensor above the frame center
    kv::geo_point const sensor_location = get_sensor_location( metadata );
    double const sensor_altitude = sensor_location.location()[ 2 ];
    kv::geo_point const frame_center = get_frame_center( metadata );
    double const frame_center_altitude = frame_center.location()[ 2 ];
    double const altitude_difference = sensor_altitude - frame_center_altitude;

    slant_range = altitude_difference / std::sin( -pitch );
  }

  return slant_range;
}

// ----------------------------------------------------------------------------
double
compute_gsd( kwiver::vital::metadata_sptr const& metadata,
             size_t frame_width, size_t frame_height )
{
  if ( frame_width == 0 || frame_height == 0 ) {
    VITAL_THROW( kv::invalid_value, "frame dimensions cannot be zero" );
  }

  kv::vector_2d sensor_fov;
  try
  {
    // Attempt to acquire sensor FOV
    sensor_fov = get_sensor_fov( metadata );
  }
  catch ( kv::invalid_value const& e )
  {
    // Fall back to calculating GSD from target width
    double const target_width = get_target_width( metadata );

    return target_width / frame_width;
  }

  // Intermediate Values
  kv::rotation_d const total_rotation = get_total_rotation( metadata );
  double yaw, pitch, roll;
  total_rotation.get_yaw_pitch_roll( yaw, pitch, roll );

  // Ideally slant range should be pre-calculated
  double const slant_range = compute_slant_range( metadata );
  double const altitude_difference = slant_range * sin( -pitch );

  // Approximate dimensions of image on ground plane
  double const target_width = 2.0 * slant_range *
                              std::tan( sensor_fov[ 0 ] / 2.0 );
  double const target_height =
    2.0 * ( slant_range * std::cos( pitch ) - altitude_difference *
            std::tan( kv::pi_over_2 - sensor_fov[ 1 ] / 2 ) );

  // GSD is the geometric mean of each dimensions's GSD
  // All values in meters per pixel
  double const x_gsd = target_width / frame_width;
  double const y_gsd = target_height / frame_height;
  double const gsd = std::sqrt( x_gsd * y_gsd );
  return gsd;
}

// ----------------------------------------------------------------------------
// Compute Video NIIRS image quality measure
// Estimation based on General Image Quality Equation v5 (GIQE5)
// See https://gwg.nga.mil/ntb/baseline/docs/GIQE-5_for_Public_Release.pdf
double
compute_vniirs( double gsd, double rer, double snr )
{
  // Taken from Table 2
  constexpr double a0 = 9.57, a1 = -3.32, a2 = 3.32,
                   a3 = -1.9, a4 = -2.0,  a5 = -1.8;

  constexpr double meters_to_inches = 1.0 / 0.0254;
  gsd *= meters_to_inches;

  double const log10_gsd = std::log10( gsd );
  double const log10_rer = std::log10( rer );

  auto vniirs = a0 +
                a1 * log10_gsd +
                a2 * ( 1.0 - std::exp( a3 / snr ) ) * log10_rer +
                a4 * std::pow( log10_rer, 4 ) +
                a5 / snr;
  // 2.0 is defined as the lower bound for VNIIRs
  vniirs = std::max( vniirs, 2.0 );
  return vniirs;
}

// ----------------------------------------------------------------------------
double
compute_rer()
{
  // TODO: Implement
  return 0.3; // Dummy value within reasonable range
}

// ----------------------------------------------------------------------------
double
compute_snr()
{
  // TODO: Implement
  return 15.0; // Dummy value within reasonable range
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
kwiver::vital::metadata_vector
compute_derived_metadata( kwiver::vital::metadata_vector const& metadata_vec,
                          size_t frame_width, size_t frame_height )
{
  kv::metadata_vector updated_values;

  for( auto const metadata : metadata_vec )
  {
    // Deep copy metadata
    auto updated_metadata =
      std::make_shared< kv::metadata >( metadata->deep_copy() );

    try
    {
      // Compute slant range. Must be inserted before GSD calculation
      auto const slant_range_value = compute_slant_range( updated_metadata );
      auto const& slant_range_trait =
        meta_traits.find( kv::VITAL_META_SLANT_RANGE );
      updated_metadata->add(
        slant_range_trait.create_metadata_item( slant_range_value ) );

      // Compute GSD
      auto const gsd_value =
        compute_gsd( updated_metadata, frame_width, frame_height );
      auto const& gsd_trait = meta_traits.find( kv::VITAL_META_AVERAGE_GSD );
      updated_metadata->add( gsd_trait.create_metadata_item( gsd_value ) );

      // Compute VNIIRS
      auto const rer_value = compute_rer();
      auto const snr_value = compute_snr();
      auto const vniirs_value = compute_vniirs( gsd_value, rer_value,
                                                snr_value );
      auto const& vniirs_trait = meta_traits.find( kv::VITAL_META_VNIIRS );
      updated_metadata->add(
        vniirs_trait.create_metadata_item( vniirs_value ) );
    }
    catch ( kv::invalid_value const& e )
    {
      // Fail silently
    }

    updated_values.push_back( updated_metadata );
  }

  return updated_values;
}

} // namespace core

} // namespace arrows

} // namespace kwiver
