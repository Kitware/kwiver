// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/**
 * \file
 * \brief Compute derived metadata fields.
 */

#include <arrows/core/derive_metadata.h>
#include <vital/types/geo_point.h>
#include <vital/types/metadata.h>
#include <vital/types/metadata_tags.h>
#include <vital/types/metadata_traits.h>

#include <cmath>

namespace kv = kwiver::vital;
using namespace kwiver::vital::algo;

namespace kwiver {

namespace arrows {

namespace core {

namespace {

static ::kwiver::vital::metadata_traits meta_traits;

double
deg2rad( double deg )
{
  return deg * M_PI / 180.0;
}

double
rad2deg( double rad )
{
  return rad * 180.0 / M_PI;
}

/*
 * Rotations needed to compute view angle
 */
static double*
RotRoll( double* in, double theta, double* out )
{
  double in0 = in[ 0 ];
  double in1 = in[ 1 ];
  double in2 = in[ 2 ];
  double sint, cost;

  theta = deg2rad( theta );
  sint = sin( theta );
  cost = cos( theta );

  out[ 0 ] = in0;
  out[ 1 ] = cost * in1 + sint * in2;
  out[ 2 ] = -sint * in1 + cost * in2;

  return out;
}

static double*
RotPitch( double* in, double theta, double* out )
{
  double in0 = in[ 0 ];
  double in1 = in[ 1 ];
  double in2 = in[ 2 ];
  double sint, cost;

  theta = deg2rad( theta );
  sint = sin( theta );
  cost = cos( theta );

  out[ 0 ] = cost * in0 + -sint * in2;
  out[ 1 ] = in1;
  out[ 2 ] = sint * in0 + cost * in2;

  return out;
}

static double*
RotYaw( double* in, double theta, double* out )
{
  double in0 = in[ 0 ];
  double in1 = in[ 1 ];
  double in2 = in[ 2 ];
  double sint, cost;

  theta = deg2rad( theta );
  sint = sin( theta );
  cost = cos( theta );

  out[ 0 ] = cost * in0 + sint * in1;
  out[ 1 ] = -sint * in0 + cost * in1;
  out[ 2 ] = in2;

  return out;
}

static double*
RotYPR( double* in, double yaw, double pitch, double roll, double* out )
{
  return RotYaw( RotPitch( RotRoll( in, roll, out ), pitch, out ), yaw, out );
}

static double
view_pitch(
  double platform_pitch,
  double platform_roll,
  double sensor_rel_az,
  double sensor_rel_el,
  double sensor_rel_roll )
{
  double forward[] = { 1, 0, 0 };
  double forward_unrotated[ 3 ];
  double right[] = { 0, 1, 0 };
  double right_unrotated[ 3 ];

  /* Start with camera view as unit vector, back out all rotations of sensor
   * and platform
   *  Initial frame of reference, X axis is perpendicular to image plane
   * (depth), Y is horizontal axis of image, Z is vertical axis of image
   *  Final frame of reference, X axis is forward direction relateive to
   * platform, Y axis is perpendicular to platform (direction of wingspan),
   *  and Z is up/down relative to platform.  Abolute view pitch theta
   * (negative below horizon) is sin(theta) = -Z/(unit), theta = arcsin(Z)
   */

  RotYPR( RotYPR( forward, -sensor_rel_az, -sensor_rel_el, -sensor_rel_roll,
                  forward_unrotated ), 0, -platform_pitch, -platform_roll,
          forward_unrotated );
  RotYPR( RotYPR( right, -sensor_rel_az, -sensor_rel_el, -sensor_rel_roll,
                  right_unrotated ), 0, -platform_pitch, -platform_roll,
          right_unrotated );

  return rad2deg( asin( -forward_unrotated[ 2 ] ) );
}

double
compute_gsd( int rows, int cols, kwiver::vital::metadata_sptr md )
{
  if( rows < 1 || cols < 1 )
  {
    return 0;
  }

  /* RP1201 method */
  auto const platform_pitch_angle = md->find(
    kv::VITAL_META_PLATFORM_PITCH_ANGLE ).as_double();
  auto const platform_roll_angle =
    md->find( kv::VITAL_META_PLATFORM_ROLL_ANGLE ).as_double();
  auto const sensor_relative_azimuth_angle = md->find(
    kv::VITAL_META_SENSOR_REL_AZ_ANGLE ).as_double();
  auto const sensor_relative_elevation_angle = md->find(
    kv::VITAL_META_SENSOR_REL_EL_ANGLE ).as_double();
  auto const sensor_relative_roll_angle = md->find(
    kv::VITAL_META_SENSOR_REL_ROLL_ANGLE ).as_double();
  auto const fov_h_deg =
    md->find( kv::VITAL_META_SENSOR_HORIZONTAL_FOV ).as_double();
  auto const fov_v_deg =
    md->find( kv::VITAL_META_SENSOR_VERTICAL_FOV ).as_double();
  auto slant_range = md->find( kv::VITAL_META_SLANT_RANGE ).as_double();
  auto const sensor_location = kv::any_cast< kv::geo_point >(
    md->find( kv::VITAL_META_SENSOR_LOCATION ).data() );
  auto const sensor_altitude = sensor_location.location()[ 2 ];
  auto const frame_center = kv::any_cast< kv::geo_point >(
    md->find( kv::VITAL_META_SENSOR_LOCATION ).data() );
  auto const frame_center_elevation = frame_center.location()[ 2 ];
  auto const target_width =
    md->find( kv::VITAL_META_TARGET_WIDTH ).as_double();

  if( platform_pitch_angle >= -20 && platform_pitch_angle <= 20 &&
      platform_roll_angle >= -50 && platform_roll_angle <= 50 &&
      sensor_relative_azimuth_angle >= 0 &&
      sensor_relative_azimuth_angle <= 360 &&
      sensor_relative_elevation_angle >= -180 &&
      sensor_relative_elevation_angle <= 180 &&
      sensor_relative_roll_angle >= 0 && sensor_relative_roll_angle <= 360 &&
      fov_h_deg >= 0 && fov_h_deg <= 180 &&
      fov_v_deg >= 0 && fov_v_deg <= 180 &&
      slant_range > 0 && slant_range <= 5000000 )
  {
    double gsd_horiz, gsd_vert;

    double const absolute_elevation_angle = view_pitch(
      platform_pitch_angle,
      platform_roll_angle,
      sensor_relative_azimuth_angle,
      sensor_relative_elevation_angle,
      sensor_relative_roll_angle );

    // Convert angles to radians
    auto const fov_h_rad = deg2rad( fov_h_deg );
    auto const fov_v_rad = deg2rad( fov_v_deg );
    auto const elev_rad = deg2rad( absolute_elevation_angle );

    // If Slant Range is missing, compute from Elevation Angle and Sensor
    // Altitude */
    if( ( slant_range == 0 || std::isnan( slant_range ) ) &&
        sensor_altitude >= -900 && sensor_altitude <= 19000 &&
        frame_center_elevation >= -900 && frame_center_elevation <=
        19000 )
    {
      slant_range = ( sensor_altitude - frame_center_elevation ) /
                    sin( -elev_rad );
    }

    // GSD horizontal
    gsd_horiz = 2.0 * slant_range * tan( ( fov_h_rad / 2.0 ) / cols );

    // GSD vertical
    gsd_vert  = ( 2.0 * slant_range * tan( ( fov_v_rad / 2.0 ) / rows ) ) /
                sin( -elev_rad );

    // GSD = Geometric mean of horiz & vert GSD
    return 1000.0 * sqrt( gsd_horiz * gsd_vert );
  }

  /* RP 1201 horizontal axis only */

  if( slant_range > 0 && slant_range <= 5000000 &&
      fov_h_deg >= 0 && fov_h_deg <= 180 )
  {
    return 2000.0 * slant_range *
           tan( ( deg2rad( fov_h_deg ) / 2.0 ) / (float) cols );
  }

  /* Target width horizontal axis only */
  if( target_width > 0 && target_width <= 10000 )
  {
    return static_cast< float >( target_width ) /
           static_cast< float >( cols ) * 1000.0;
  }

  return 0;
}

// General Image Quality Equation v5 (GIQE5)
static double
giqe5( double gsd )
{
  constexpr double A0 = 9.57;
  constexpr double A1 = -3.32;

  auto const vniirs = A0 + A1 * log10( gsd / 25.4 );
  return vniirs;
}

} // namespace

kwiver::vital::metadata_vector
compute_derived_metadata( kwiver::vital::metadata_vector metadata_vec,
                          int frame_width, int frame_height )
{
  for( auto const md : metadata_vec )
  {
    auto const gsd_value = compute_gsd( frame_height, frame_width, md );
    auto vniirs_value = giqe5( gsd_value );
    vniirs_value = std::max( 2.0, vniirs_value );

    const auto& vniirs_trait = meta_traits.find( kv::VITAL_META_VNIIRS );
    const auto& gsd_trait = meta_traits.find( kv::VITAL_META_AVERAGE_GSD );

    // Add the new fields
    md->add( vniirs_trait.create_metadata_item( vniirs_value ) );
    md->add( gsd_trait.create_metadata_item( gsd_value ) );
  }
  return metadata_vec;
}

} // end namespace core

} // end namespace arrows

} // end namespace kwiver
