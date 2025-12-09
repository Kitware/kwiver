// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of derive_corner_points algorithm.

#include <arrows/geocalc/algo/derive_corner_points.h>

#include <arrows/geocalc/geo_conv.h>
#include <arrows/geocalc/projection.h>

#include <vital/logger/logger.h>
#include <vital/math_constants.h>
#include <vital/types/geodesy.h>
#include <vital/types/rotation.h>

#include <optional>

namespace kwiver {

namespace arrows {

namespace geocalc {

namespace {

// ----------------------------------------------------------------------------
template < class T >
bool
is_valid( vital::metadata_item const& item )
{
  if( !item )
  {
    return false;
  }

  if constexpr( std::is_same_v< T, double > )
  {
    return std::isfinite( item.get< double >() );
  }
  else if constexpr( std::is_same_v< T, vital::geo_point > )
  {
    return item.get< vital::geo_point >().location().allFinite();
  }
  else if constexpr( std::is_same_v< T, vital::geo_polygon > )
  {
    auto const points =
      item.get< vital::geo_polygon >()
      .polygon()
      .get_vertices();

    // At least one of the four corner points must not be NaN
    return
      points.size() == 4 &&
      std::any_of(
      points.begin(), points.end(),
      []( vital::vector_2d const& p ){ return p.allFinite(); } );
  }

  return true;
}

// ----------------------------------------------------------------------------
std::optional< vital::geo_polygon >
derive_corner_points_from_frame(
  vital::metadata const& metadata,
  double default_altitude )
{
  geo_conversion converter;

  // First see if we have all the requisite components
  auto const& sensor_location_item =
    metadata.find( vital::VITAL_META_SENSOR_LOCATION );
  auto const& frame_center_item =
    metadata.find( vital::VITAL_META_FRAME_CENTER );
  auto const& sensor_yaw_item =
    metadata.find( vital::VITAL_META_SENSOR_REL_AZ_ANGLE );
  auto const& sensor_pitch_item =
    metadata.find( vital::VITAL_META_SENSOR_REL_EL_ANGLE );
  auto const& sensor_roll_item =
    metadata.find( vital::VITAL_META_SENSOR_REL_ROLL_ANGLE );
  auto const& sensor_abs_yaw_item =
    metadata.find( vital::VITAL_META_SENSOR_YAW_ANGLE );
  auto const& sensor_abs_pitch_item =
    metadata.find( vital::VITAL_META_SENSOR_PITCH_ANGLE );
  auto const& sensor_abs_roll_item =
    metadata.find( vital::VITAL_META_SENSOR_ROLL_ANGLE );
  auto const& platform_yaw_item =
    metadata.find( vital::VITAL_META_PLATFORM_HEADING_ANGLE );
  auto const& platform_pitch_item =
    metadata.find( vital::VITAL_META_PLATFORM_PITCH_ANGLE );
  auto const& platform_roll_item =
    metadata.find( vital::VITAL_META_PLATFORM_ROLL_ANGLE );
  auto const& sensor_hfov_item =
    metadata.find( vital::VITAL_META_SENSOR_HORIZONTAL_FOV );
  auto const& sensor_vfov_item =
    metadata.find( vital::VITAL_META_SENSOR_VERTICAL_FOV );

  auto const abs_sensor_rotation_available =
    is_valid< double >( sensor_abs_yaw_item ) &&
    is_valid< double >( sensor_abs_pitch_item ) &&
    is_valid< double >( sensor_abs_roll_item );
  auto const rel_sensor_rotation_available =
    is_valid< double >( sensor_yaw_item ) &&
    is_valid< double >( platform_yaw_item ) &&
    is_valid< double >( sensor_pitch_item ) &&
    is_valid< double >( platform_pitch_item ) &&
    is_valid< double >( sensor_roll_item ) &&
    is_valid< double >( platform_roll_item );

  if(
    !is_valid< vital::geo_point >( sensor_location_item ) ||
    ( !abs_sensor_rotation_available && !rel_sensor_rotation_available ) ||
    !is_valid< double >( sensor_hfov_item ) ||
    !is_valid< double >( sensor_vfov_item ) )
  {
    return std::nullopt;
  }

  auto const& sensor_location =
    sensor_location_item.get< vital::geo_point >();
  auto const sensor_geodetic_location =
    sensor_location.location( vital::SRID::lat_lon_WGS84 );
  auto const sensor_ecef_location =
    sensor_location.location( vital::SRID::ECEF_WGS84 );

  double altitude = default_altitude;
  if( is_valid< vital::geo_point >( frame_center_item ) )
  {
    altitude = frame_center_item.get< vital::geo_point >().location()[ 2 ];
  }
  else
  {
    LOG_WARN(
      vital::get_logger( "arrows.geocalc.derive_corner_points" ),
      "Frame center is not valid, using default altitude: " <<
        default_altitude );
  }

  // Combine rotations and convert to proper coordinate system
  vital::rotation_d geodetic_rotation;
  if( abs_sensor_rotation_available )
  {
    geodetic_rotation =
      vital::rotation_d{
      sensor_abs_yaw_item.as_double() * vital::deg_to_rad,
      sensor_abs_pitch_item.as_double() * vital::deg_to_rad,
      sensor_abs_roll_item.as_double() * vital::deg_to_rad };
  }
  else
  {
    auto const platform_rotation =
      vital::rotation_d{
      platform_yaw_item.as_double() * vital::deg_to_rad,
      platform_pitch_item.as_double() * vital::deg_to_rad,
      platform_roll_item.as_double() * vital::deg_to_rad };
    auto const sensor_rel_rotation =
      vital::rotation_d{
      sensor_yaw_item.as_double() * vital::deg_to_rad,
      sensor_pitch_item.as_double() * vital::deg_to_rad,
      sensor_roll_item.as_double() * vital::deg_to_rad };
    geodetic_rotation = platform_rotation * sensor_rel_rotation;
  }

  auto const geodetic_to_ecef_rotation = vital::rotation_d{
    sensor_geodetic_location[ 0 ] * vital::deg_to_rad,
    -( sensor_geodetic_location[ 1 ] * vital::deg_to_rad + vital::pi_over_2 ),
    0.0 };
  auto const ecef_rotation = geodetic_to_ecef_rotation * geodetic_rotation;

  auto const hfov = sensor_hfov_item.as_double() * vital::deg_to_rad / 2.0;
  auto const vfov = sensor_vfov_item.as_double() * vital::deg_to_rad / 2.0;

  std::vector< vital::rotation_d > const corner_rotations = {
    vital::rotation_d{ -hfov, vfov, 0.0 },
    vital::rotation_d{ hfov, vfov, 0.0 },
    vital::rotation_d{ hfov, -vfov, 0.0 },
    vital::rotation_d{ -hfov, -vfov, 0.0 }, };

  // Project each corner to ground plane
  std::vector< vital::vector_2d > corners;
  for( auto const& corner_rotation : corner_rotations )
  {
    try
    {
      if( auto const corner_ecef =
            raycast_ecef_to_ellipsoid(
              sensor_ecef_location,
              ecef_rotation * corner_rotation * vital::vector_3d{ 1, 0, 0 },
              vital::SRID::ECEF_WGS84,
              altitude ) )
      {
        auto const corner_geodetic =
          converter(
            *corner_ecef, vital::SRID::ECEF_WGS84,
            vital::SRID::lat_lon_WGS84 );
        corners.emplace_back( corner_geodetic[ 0 ], corner_geodetic[ 1 ] );
        continue;
      }
    }
    catch( std::runtime_error const& )
    {
      // Fall through
    }

    corners.emplace_back(
      std::numeric_limits< double >::quiet_NaN(),
      std::numeric_limits< double >::quiet_NaN() );
  }

  return vital::geo_polygon{
    vital::polygon{ corners }, vital::SRID::lat_lon_WGS84 };
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
vital::metadata_vector
derive_corner_points
::filter(
  vital::metadata_vector const& input_metadata,
  VITAL_UNUSED vital::image_container_scptr const& input_image )
{
  vital::metadata_vector updated_values;
  for( auto const& metadata : input_metadata )
  {
    auto updated_metadata = vital::metadata_sptr( metadata->clone() );
    updated_values.push_back( updated_metadata );

    // Possibly skip if corner point data already exists
    if( !c_overwrite && metadata->has( vital::VITAL_META_CORNER_POINTS ) &&
        is_valid< vital::geo_polygon >(
          metadata->find( vital::VITAL_META_CORNER_POINTS ) ) )
    {
      continue;
    }

    if( auto corner_polygon =
          derive_corner_points_from_frame( *metadata, c_default_altitude ) )
    {
      updated_metadata->add< vital::VITAL_META_CORNER_POINTS >(
        *corner_polygon );
    }
  }

  return updated_values;
}

// ----------------------------------------------------------------------------
bool
derive_corner_points
::check_configuration( VITAL_UNUSED vital::config_block_sptr config ) const
{
  return true;
}

// ----------------------------------------------------------------------------
void
derive_corner_points
::initialize()
{
  this->set_capability( CAN_USE_FRAME_IMAGE, false );
}

} // namespace geocalc

} // namespace arrows

} // namespace kwiver
