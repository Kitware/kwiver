// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of KLV-vital conversion functions.

#include "klv_convert_vital.h"

#include "klv_0102.h"
#include "klv_0104.h"
#include "klv_0601.h"
#include "klv_1108.h"
#include "klv_1108_metric_set.h"

#include <vital/range/iota.h>
#include <vital/types/geodesy.h>
#include <vital/types/metadata_types.h>

#include <iomanip>

#include <cstdint>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

// ----------------------------------------------------------------------------
struct klv_to_vital_visitor
{
  template < class T,
             typename std::enable_if< std::is_same< T, uint64_t >::value ||
                                      std::is_same< T, double >::value ||
                                      std::is_same< T, std::string >::value,
                                      bool >::type = true >
  kv::metadata_value
  operator()() const
  {
    return value.get< T >();
  }

  template < class T,
             typename std::enable_if< !std::is_same< T, uint64_t >::value &&
                                      !std::is_same< T, double >::value &&
                                      !std::is_same< T, std::string >::value,
                                      bool >::type = true >
  kv::metadata_value
  operator()() const
  {
    throw std::logic_error( "type does not exist in klv_value" );
  }

  klv_value const& value;
};

// ----------------------------------------------------------------------------
kv::metadata_value
klv_to_vital_value( klv_value const& value )
{
  return kv::visit_metadata_types_return< kv::metadata_value >(
    klv_to_vital_visitor{ value }, value.type() );
}

// ----------------------------------------------------------------------------
// Create a geo_point with invalid values replaced with NaN
kv::geo_point
assemble_geo_point( klv_value const& latitude,
                    klv_value const& longitude,
                    klv_value const& elevation )
{
  constexpr auto qnan = std::numeric_limits< double >::quiet_NaN();
  return {
    kv::vector_3d{
      longitude.valid() ? longitude.get< double >() : qnan,
      latitude.valid() ? latitude.get< double >() : qnan,
      elevation.valid() ? elevation.get< double >() : qnan, },
    kv::SRID::lat_lon_WGS84 };
}

// ----------------------------------------------------------------------------
// Create a geo_point from the given lists of tags, which are queried in order
// to enforce precedence of e.g. newer or more precise tags over deprecated or
// less precise ones
kv::optional< kv::geo_point >
parse_geo_point( klv_timeline const& klv_data,
                 klv_top_level_tag standard,
                 uint64_t timestamp,
                 std::vector< klv_lds_key > const& latitude_tags,
                 std::vector< klv_lds_key > const& longitude_tags,
                 std::vector< klv_lds_key > const& elevation_tags )
{
  klv_value latitude;
  for( auto const tag : latitude_tags )
  {
    latitude = klv_data.at( standard, tag, timestamp );
    if( latitude.valid() )
    {
      break;
    }
  }
  if( !latitude.valid() )
  {
    return kv::nullopt;
  }

  klv_value longitude;
  for( auto const tag : longitude_tags )
  {
    longitude = klv_data.at( standard, tag, timestamp );
    if( longitude.valid() )
    {
      break;
    }
  }
  if( !longitude.valid() )
  {
    return kv::nullopt;
  }

  klv_value elevation;
  for( auto const tag : elevation_tags )
  {
    elevation = klv_data.at( standard, tag, timestamp );
    if( elevation.valid() )
    {
      break;
    }
  }

  return assemble_geo_point( latitude, longitude, elevation );
}

// ----------------------------------------------------------------------------
void
klv_0104_parse_datetime_to_unix(
  klv_timeline const& klv_data, uint64_t timestamp, kv::metadata& vital_data,
  klv_lds_key klv_tag, kv::vital_metadata_tag vital_tag )
{
  constexpr auto standard = KLV_PACKET_MISB_0104_UNIVERSAL_SET;

  auto const datetime = klv_data.at( standard, klv_tag, timestamp );
  if( datetime.valid() )
  {
    try
    {
      auto const value = datetime.get< std::string >();
      vital_data.add(
        vital_tag, kv::std_0104_datetime_to_unix_timestamp( value ) );
    }
    catch( kv::metadata_exception const& e )
    {
      LOG_ERROR( kv::get_logger( "klv" ), e.what() );
    }
  }

}

// ----------------------------------------------------------------------------
void
klv_0102_to_vital_metadata( klv_timeline const& klv_data, uint64_t timestamp,
                            kv::metadata& vital_data )
{
  constexpr auto standard = KLV_PACKET_MISB_0102_LOCAL_SET;

  // Add the timestamp
  vital_data.add< kv::VITAL_META_UNIX_TIMESTAMP >( timestamp );

  // Check if there is a ST0102 embedded in ST0601
  auto const st0601 =
    klv_data.at( KLV_PACKET_MISB_0601_LOCAL_SET,
                 KLV_0601_SECURITY_LOCAL_SET, timestamp );

  // Get the tag from any ST0102 source
  auto const get_tag_value =
    [ & ]( klv_0102_tag tag ){
      auto result = klv_data.at( standard, tag, timestamp );
      if( !result.valid() && st0601.valid() )
      {
        auto const& st0601_set = st0601.get< klv_local_set >();
        auto const it = st0601_set.find( tag );
        if( it != st0601_set.end() )
        {
          result = it->second;
        }
      }
      return result;
    };

  // Convert the security classification to a string
  {
    auto const value = get_tag_value( KLV_0102_SECURITY_CLASSIFICATION );
    if( value.valid() )
    {
      std::stringstream ss;
      ss << value.get< klv_0102_security_classification >();
      vital_data.add< kv::VITAL_META_SECURITY_CLASSIFICATION >( ss.str() );
    }
  }
}

// ----------------------------------------------------------------------------
void
klv_0104_to_vital_metadata( klv_timeline const& klv_data, uint64_t timestamp,
                            kv::metadata& vital_data )
{
  constexpr auto standard = KLV_PACKET_MISB_0104_UNIVERSAL_SET;

  // Add the timestamp
  vital_data.add< kv::VITAL_META_UNIX_TIMESTAMP >( timestamp );

  static std::map< klv_lds_key, kv::vital_metadata_tag > const direct_map = {
    { KLV_0104_PLATFORM_HEADING_ANGLE,
      kv::VITAL_META_PLATFORM_HEADING_ANGLE },
    { KLV_0104_PLATFORM_PITCH_ANGLE,
      kv::VITAL_META_PLATFORM_PITCH_ANGLE },
    { KLV_0104_PLATFORM_ROLL_ANGLE,
      kv::VITAL_META_PLATFORM_ROLL_ANGLE },
    { KLV_0104_DEVICE_DESIGNATION,
      kv::VITAL_META_PLATFORM_DESIGNATION },
    { KLV_0104_IMAGE_SOURCE_DEVICE,
      kv::VITAL_META_IMAGE_SOURCE_SENSOR },
    { KLV_0104_IMAGE_COORDINATE_SYSTEM,
      kv::VITAL_META_IMAGE_COORDINATE_SYSTEM },
    { KLV_0104_HORIZONTAL_FOV,
      kv::VITAL_META_SENSOR_HORIZONTAL_FOV },
    { KLV_0104_VERTICAL_FOV,
      kv::VITAL_META_SENSOR_VERTICAL_FOV },
    { KLV_0104_SLANT_RANGE,
      kv::VITAL_META_SLANT_RANGE },
    { KLV_0104_TARGET_WIDTH,
      kv::VITAL_META_TARGET_WIDTH },
    { KLV_0104_SENSOR_ROLL_ANGLE,
      kv::VITAL_META_SENSOR_ROLL_ANGLE },
    { KLV_0104_ANGLE_TO_NORTH,
      kv::VITAL_META_ANGLE_TO_NORTH },
    { KLV_0104_OBLIQUITY_ANGLE,
      kv::VITAL_META_OBLIQUITY_ANGLE }, };

  // Convert all the direct mappings en masse
  for( auto const& entry : direct_map )
  {
    auto const klv_tag = entry.first;
    auto const vital_tag = entry.second;
    auto const value = klv_data.at( standard, klv_tag, timestamp );
    if( value.valid() )
    {
      auto const converted_value = klv_to_vital_value( value );
      vital_data.add( vital_tag, converted_value );
    }
  }

  // Convert the episode/mission number (an actual number here) to a string
  auto const episode_number =
    klv_data.at( standard, KLV_0104_EPISODE_NUMBER, timestamp );
  if( episode_number.valid() )
  {
    auto const value = episode_number.get< double >();
    std::stringstream ss;
    ss << std::fixed << value;
    vital_data.add< kv::VITAL_META_MISSION_NUMBER >( ss.str() );
  }

  // Parse the datetime strings into UNIX microsecond timestamps
  klv_0104_parse_datetime_to_unix(
    klv_data, timestamp, vital_data,
    KLV_0104_START_DATETIME, kv::VITAL_META_START_TIMESTAMP );
  klv_0104_parse_datetime_to_unix(
    klv_data, timestamp, vital_data,
    KLV_0104_EVENT_START_DATETIME, kv::VITAL_META_EVENT_START_TIMESTAMP );

  // Sensor location
  auto const sensor_location =
    parse_geo_point( klv_data, standard, timestamp,
                     { KLV_0104_DEVICE_LATITUDE },
                     { KLV_0104_DEVICE_LONGITUDE },
                     { KLV_0104_DEVICE_ALTITUDE } );
  if( sensor_location )
  {
    vital_data.add< kv::VITAL_META_SENSOR_LOCATION >( *sensor_location );
  }

  // Frame center location
  auto const frame_center_location =
    parse_geo_point( klv_data, standard, timestamp,
                     { KLV_0104_FRAME_CENTER_LATITUDE },
                     { KLV_0104_FRAME_CENTER_LONGITUDE },
                     { KLV_0104_FRAME_CENTER_ELEVATION } );
  if( frame_center_location )
  {
    vital_data.add< kv::VITAL_META_FRAME_CENTER >( *frame_center_location );
  }

  // Image frame corner point locations
  std::vector< kv::optional< kv::geo_point > > corner_points = {
    parse_geo_point( klv_data, standard, timestamp,
                     { KLV_0104_CORNER_LATITUDE_POINT_1 },
                     { KLV_0104_CORNER_LONGITUDE_POINT_1 },
                     {} ),
    parse_geo_point( klv_data, standard, timestamp,
                     { KLV_0104_CORNER_LATITUDE_POINT_2 },
                     { KLV_0104_CORNER_LONGITUDE_POINT_2 },
                     {} ),
    parse_geo_point( klv_data, standard, timestamp,
                     { KLV_0104_CORNER_LATITUDE_POINT_3 },
                     { KLV_0104_CORNER_LONGITUDE_POINT_3 },
                     {} ),
    parse_geo_point( klv_data, standard, timestamp,
                     { KLV_0104_CORNER_LATITUDE_POINT_4 },
                     { KLV_0104_CORNER_LONGITUDE_POINT_4 },
                     {} ), };

  // Add the frame corners if we found all of them
  if( std::all_of( corner_points.cbegin(), corner_points.cend(),
                   []( kv::optional< kv::geo_point > const& value ) -> bool {
                     return value.has_value();
                   } ) )
  {
    std::vector< kv::vector_2d > points;
    for( auto const i : kv::range::iota< size_t >( 4 ) )
    {
      points.emplace_back(
          corner_points[ i ]->location( kv::SRID::lat_lon_WGS84 ).head< 2 >() );
    }

    auto const polygon = kv::geo_polygon{ points, kv::SRID::lat_lon_WGS84 };
    vital_data.add< kv::VITAL_META_CORNER_POINTS >( polygon );
  }
}

// ----------------------------------------------------------------------------
void
klv_0601_to_vital_metadata( klv_timeline const& klv_data, uint64_t timestamp,
                            kv::metadata& vital_data )
{
  constexpr auto standard = KLV_PACKET_MISB_0601_LOCAL_SET;

  // Add the timestamp
  vital_data.add< kv::VITAL_META_UNIX_TIMESTAMP >( timestamp );

  static std::map< klv_lds_key, kv::vital_metadata_tag > const direct_map = {
    { KLV_0601_MISSION_ID,
      kv::VITAL_META_MISSION_ID },
    { KLV_0601_PLATFORM_TAIL_NUMBER,
      kv::VITAL_META_PLATFORM_TAIL_NUMBER },
    { KLV_0601_PLATFORM_HEADING_ANGLE,
      kv::VITAL_META_PLATFORM_HEADING_ANGLE },
    { KLV_0601_PLATFORM_PITCH_ANGLE,
      kv::VITAL_META_PLATFORM_PITCH_ANGLE },
    { KLV_0601_PLATFORM_ROLL_ANGLE,
      kv::VITAL_META_PLATFORM_ROLL_ANGLE },
    { KLV_0601_PLATFORM_TRUE_AIRSPEED,
      kv::VITAL_META_PLATFORM_TRUE_AIRSPEED },
    { KLV_0601_PLATFORM_INDICATED_AIRSPEED,
      kv::VITAL_META_PLATFORM_INDICATED_AIRSPEED },
    { KLV_0601_PLATFORM_DESIGNATION,
      kv::VITAL_META_PLATFORM_DESIGNATION },
    { KLV_0601_IMAGE_SOURCE_SENSOR,
      kv::VITAL_META_IMAGE_SOURCE_SENSOR },
    { KLV_0601_IMAGE_COORDINATE_SYSTEM,
      kv::VITAL_META_IMAGE_COORDINATE_SYSTEM },
    { KLV_0601_SENSOR_HORIZONTAL_FOV,
      kv::VITAL_META_SENSOR_HORIZONTAL_FOV },
    { KLV_0601_SENSOR_VERTICAL_FOV,
      kv::VITAL_META_SENSOR_VERTICAL_FOV },
    { KLV_0601_SENSOR_RELATIVE_AZIMUTH_ANGLE,
      kv::VITAL_META_SENSOR_REL_AZ_ANGLE },
    { KLV_0601_SENSOR_RELATIVE_ELEVATION_ANGLE,
      kv::VITAL_META_SENSOR_REL_EL_ANGLE },
    { KLV_0601_SENSOR_RELATIVE_ROLL_ANGLE,
      kv::VITAL_META_SENSOR_REL_ROLL_ANGLE },
    { KLV_0601_SLANT_RANGE,
      kv::VITAL_META_SLANT_RANGE },
    { KLV_0601_TARGET_WIDTH,
      kv::VITAL_META_TARGET_WIDTH },
    { KLV_0601_TARGET_WIDTH_EXTENDED,
      kv::VITAL_META_TARGET_WIDTH },
    { KLV_0601_STATIC_PRESSURE,
      kv::VITAL_META_STATIC_PRESSURE },
    { KLV_0601_DENSITY_ALTITUDE,
      kv::VITAL_META_DENSITY_ALTITUDE },
    { KLV_0601_DENSITY_ALTITUDE_EXTENDED,
      kv::VITAL_META_DENSITY_ALTITUDE },
    { KLV_0601_OUTSIDE_AIR_TEMPERATURE,
      kv::VITAL_META_OUTSIDE_AIR_TEMPERATURE },
    { KLV_0601_TARGET_TRACK_GATE_WIDTH,
      kv::VITAL_META_TARGET_TRK_GATE_WIDTH },
    { KLV_0601_TARGET_TRACK_GATE_HEIGHT,
      kv::VITAL_META_TARGET_TRK_GATE_HEIGHT },
    { KLV_0601_TARGET_ERROR_ESTIMATE_CE90,
      kv::VITAL_META_TARGET_ERROR_EST_CE90 },
    { KLV_0601_TARGET_ERROR_ESTIMATE_LE90,
      kv::VITAL_META_TARGET_ERROR_EST_LE90 },
    { KLV_0601_DIFFERENTIAL_PRESSURE,
      kv::VITAL_META_DIFFERENTIAL_PRESSURE },
    { KLV_0601_PLATFORM_ANGLE_OF_ATTACK,
      kv::VITAL_META_PLATFORM_ANG_OF_ATTACK },
    { KLV_0601_PLATFORM_VERTICAL_SPEED,
      kv::VITAL_META_PLATFORM_VERTICAL_SPEED },
    { KLV_0601_PLATFORM_SIDESLIP_ANGLE,
      kv::VITAL_META_PLATFORM_SIDESLIP_ANGLE },
    { KLV_0601_AIRFIELD_BAROMETRIC_PRESSURE,
      kv::VITAL_META_AIRFIELD_BAROMET_PRESS },
    { KLV_0601_AIRFIELD_ELEVATION,
      kv::VITAL_META_AIRFIELD_ELEVATION },
    { KLV_0601_RELATIVE_HUMIDITY,
      kv::VITAL_META_RELATIVE_HUMIDITY },
    { KLV_0601_PLATFORM_GROUND_SPEED,
      kv::VITAL_META_PLATFORM_GROUND_SPEED },
    { KLV_0601_GROUND_RANGE,
      kv::VITAL_META_GROUND_RANGE },
    { KLV_0601_PLATFORM_FUEL_REMAINING,
      kv::VITAL_META_PLATFORM_FUEL_REMAINING },
    { KLV_0601_PLATFORM_CALL_SIGN,
      kv::VITAL_META_PLATFORM_CALL_SIGN },
    { KLV_0601_LASER_PRF_CODE,
      kv::VITAL_META_LASER_PRF_CODE },
    { KLV_0601_PLATFORM_MAGNETIC_HEADING,
      kv::VITAL_META_PLATFORM_MAGNET_HEADING },
    { KLV_0601_EVENT_START_TIME,
      kv::VITAL_META_EVENT_START_TIMESTAMP },
    { KLV_0601_VERSION_NUMBER,
      kv::VITAL_META_UAS_LDS_VERSION_NUMBER }, };

  // Convert all the direct mappings en masse
  for( auto const& entry : direct_map )
  {
    auto const klv_tag = entry.first;
    auto const vital_tag = entry.second;
    auto const value = klv_data.at( standard, klv_tag, timestamp );
    if( value.valid() )
    {
      auto const converted_value = klv_to_vital_value( value );
      vital_data.add( vital_tag, converted_value );
    }
  }

  // Convert enum to integer
  auto const icing_detected =
    klv_data.at( standard, KLV_0601_ICING_DETECTED, timestamp );
  if( icing_detected.valid() )
  {
    using value_t = kv::type_of_tag< kv::VITAL_META_ICING_DETECTED >;

    auto const value =
      static_cast< value_t >(
        icing_detected.get< klv_0601_icing_detected >() );
    vital_data.add< kv::VITAL_META_ICING_DETECTED >( value );
  }

  // Convert enum to integer
  auto const sensor_fov_name =
    klv_data.at( standard, KLV_0601_SENSOR_FOV_NAME, timestamp );
  if( sensor_fov_name.valid() )
  {
    using value_t = kv::type_of_tag< kv::VITAL_META_SENSOR_FOV_NAME >;

    auto const value =
      static_cast< value_t >(
        sensor_fov_name.get< klv_0601_sensor_fov_name >() );
    vital_data.add< kv::VITAL_META_SENSOR_FOV_NAME >( value );
  }

  // If more than these two enum -> int conversions become necessary, consider
  // creating a template function to avoid copy-paste

  // Sensor location
  auto const sensor_location =
    parse_geo_point( klv_data, standard, timestamp,
                     { KLV_0601_SENSOR_LATITUDE },
                     { KLV_0601_SENSOR_LONGITUDE },
                     { KLV_0601_SENSOR_ELLIPSOID_HEIGHT_EXTENDED,
                       KLV_0601_SENSOR_ELLIPSOID_HEIGHT,
                       KLV_0601_SENSOR_TRUE_ALTITUDE } );
  if( sensor_location )
  {
    vital_data.add< kv::VITAL_META_SENSOR_LOCATION >( *sensor_location );
  }

  // Frame center location
  auto const frame_center_location =
    parse_geo_point( klv_data, standard, timestamp,
                     { KLV_0601_FRAME_CENTER_LATITUDE },
                     { KLV_0601_FRAME_CENTER_LONGITUDE },
                     { KLV_0601_FRAME_CENTER_HEIGHT_ABOVE_ELLIPSOID,
                       KLV_0601_FRAME_CENTER_ELEVATION } );
  if( frame_center_location )
  {
    vital_data.add< kv::VITAL_META_FRAME_CENTER >( *frame_center_location );
  }

  // Target location
  auto const target_location =
    parse_geo_point( klv_data, standard, timestamp,
                     { KLV_0601_TARGET_LOCATION_LATITUDE },
                     { KLV_0601_TARGET_LOCATION_LONGITUDE },
                     { KLV_0601_TARGET_LOCATION_ELEVATION } );
  if( target_location )
  {
    vital_data.add< kv::VITAL_META_TARGET_LOCATION >( *target_location );
  }

  // Image frame corner point locations
  std::vector< kv::optional< kv::geo_point > > corner_points = {
    parse_geo_point( klv_data, standard, timestamp,
                     { KLV_0601_FULL_CORNER_LATITUDE_POINT_1 },
                     { KLV_0601_FULL_CORNER_LONGITUDE_POINT_1 },
                     {} ),
    parse_geo_point( klv_data, standard, timestamp,
                     { KLV_0601_FULL_CORNER_LATITUDE_POINT_2 },
                     { KLV_0601_FULL_CORNER_LONGITUDE_POINT_2 },
                     {} ),
    parse_geo_point( klv_data, standard, timestamp,
                     { KLV_0601_FULL_CORNER_LATITUDE_POINT_3 },
                     { KLV_0601_FULL_CORNER_LONGITUDE_POINT_3 },
                     {} ),
    parse_geo_point( klv_data, standard, timestamp,
                     { KLV_0601_FULL_CORNER_LATITUDE_POINT_4 },
                     { KLV_0601_FULL_CORNER_LONGITUDE_POINT_4 },
                     {} ), };

  // Try to assemble any missing frame corner points using the legacy tags
  if( target_location )
  {
    auto const target_location_vector =
      target_location->location( kv::SRID::lat_lon_WGS84 );
    std::vector< kv::optional< kv::geo_point > > const offset_corner_points = {
      parse_geo_point( klv_data, standard, timestamp,
                       { KLV_0601_OFFSET_CORNER_LATITUDE_POINT_1 },
                       { KLV_0601_OFFSET_CORNER_LONGITUDE_POINT_1 },
                       {} ),
      parse_geo_point( klv_data, standard, timestamp,
                       { KLV_0601_OFFSET_CORNER_LATITUDE_POINT_2 },
                       { KLV_0601_OFFSET_CORNER_LONGITUDE_POINT_2 },
                       {} ),
      parse_geo_point( klv_data, standard, timestamp,
                       { KLV_0601_OFFSET_CORNER_LATITUDE_POINT_3 },
                       { KLV_0601_OFFSET_CORNER_LONGITUDE_POINT_3 },
                       {} ),
      parse_geo_point( klv_data, standard, timestamp,
                       { KLV_0601_OFFSET_CORNER_LATITUDE_POINT_4 },
                       { KLV_0601_OFFSET_CORNER_LONGITUDE_POINT_4 },
                       {} ), };

    for( auto const i : kv::range::iota< size_t >( 4 ) )
    {
      if( !corner_points.at( i ) && offset_corner_points.at( i ) )
      {
        auto const offset_vector =
          offset_corner_points.at( i )->location( kv::SRID::lat_lon_WGS84 );
        corner_points.at( i ) =
          kv::geo_point{
          kv::vector_3d{ target_location_vector + offset_vector },
          kv::SRID::lat_lon_WGS84 };
      }
    }
  }

  // Add the frame corners if we found all of them
  if( std::all_of( corner_points.cbegin(), corner_points.cend(),
                   []( kv::optional< kv::geo_point > const& value ) -> bool {
                     return value.has_value();
                   } ) )
  {
    std::vector< kv::vector_2d > points;
    for( auto const i : kv::range::iota< size_t >( 4 ) )
    {
      points.emplace_back(
          corner_points[ i ]->location( kv::SRID::lat_lon_WGS84 ).head< 2 >() );
    }

    auto const polygon = kv::geo_polygon{ points, kv::SRID::lat_lon_WGS84 };
    vital_data.add< kv::VITAL_META_CORNER_POINTS >( polygon );
  }
}

// ----------------------------------------------------------------------------
void
klv_1108_to_vital_metadata( klv_timeline const& klv_data, uint64_t timestamp,
                            kv::metadata& vital_data )
{
  constexpr auto standard = KLV_PACKET_MISB_1108_LOCAL_SET;

  // Add the timestamp
  vital_data.add< kv::VITAL_META_UNIX_TIMESTAMP >( timestamp );

  static std::map< std::string, kv::vital_metadata_tag > const metrics = {
    { "GSD", kv::VITAL_META_AVERAGE_GSD },
    { "VNIIRS", kv::VITAL_META_VNIIRS }, };

  // Find the most recently calculated valid metric value for each supported
  // metric
  for( auto const& metric : metrics )
  {
    klv_local_set const* best_metric_set = nullptr;
    for( auto const& metric_set_entry :
         klv_data.all_at( standard, KLV_1108_METRIC_LOCAL_SET, timestamp ) )
    {
      if( !metric_set_entry.valid() )
      {
        continue;
      }

      auto const& metric_set = metric_set_entry.get< klv_local_set >();
      auto const name_entry = metric_set.at( KLV_1108_METRIC_SET_NAME );
      if( !name_entry.valid() )
      {
        continue;
      }

      auto const& name = name_entry.get< std::string >();
      if( name != metric.first )
      {
        continue;
      }

      if( !best_metric_set ||
          best_metric_set->at( KLV_1108_METRIC_SET_TIME ) <
          metric_set.at( KLV_1108_METRIC_SET_TIME ) )
      {
        best_metric_set = &metric_set;
      }
    }

    if( best_metric_set )
    {
      auto const value =
        best_metric_set->at( KLV_1108_METRIC_SET_VALUE ).get< double >();
      vital_data.add( metric.second, value );
    }
  }
}

} // namespace <anonymous>

// ----------------------------------------------------------------------------
kv::metadata_sptr
klv_to_vital_metadata( klv_timeline const& klv_data, uint64_t timestamp )
{
  auto const result = std::make_shared< kv::metadata >();
  klv_0102_to_vital_metadata( klv_data, timestamp, *result );
  klv_0104_to_vital_metadata( klv_data, timestamp, *result );
  klv_0601_to_vital_metadata( klv_data, timestamp, *result );
  klv_1108_to_vital_metadata( klv_data, timestamp, *result );
  return result;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
