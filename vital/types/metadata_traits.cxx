// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief This file contains the implementation for metadata traits.

#include "metadata_traits.h"

#include <vital/types/metadata.h>
#include <vital/util/demangle.h>

namespace kwiver {

namespace vital {

namespace {

// ----------------------------------------------------------------------------
std::vector< metadata_tag_traits > const&
tag_traits()
{
#define TRAITS_OF( X ) X, #X, typeid( type_of_tag< X > )
  static auto const array = std::vector< metadata_tag_traits >{
    { TRAITS_OF( VITAL_META_UNKNOWN ),
      "Unknown / Undefined Entry",
      "Unknown or undefined entry." },
    { TRAITS_OF( VITAL_META_METADATA_ORIGIN ),
      "Origin of Metadata",
      "Name of the metadata standard used to decode these metadata values "
      "from a video stream." },
    { TRAITS_OF( VITAL_META_UNIX_TIMESTAMP ),
      "Unix Timestamp (microseconds)",
      "Number of microseconds since the Unix epoch, not counting leap "
      "seconds." },
    { TRAITS_OF( VITAL_META_MISSION_ID ),
      "Mission ID",
      "Descriptive mission identifier to distinguish event or sortie." },
    { TRAITS_OF( VITAL_META_MISSION_NUMBER ),
      "Episode Number",
      "Episode number." },
    { TRAITS_OF( VITAL_META_PLATFORM_TAIL_NUMBER ),
      "Platform Tail Number",
      "Identifier of platform as posted." },
    { TRAITS_OF( VITAL_META_PLATFORM_HEADING_ANGLE ),
      "Platform Heading Angle (degrees)",
      "Aircraft heading angle. Relative between longitudinal axis and True "
      "North measured in the horizontal plane." },
    { TRAITS_OF( VITAL_META_PLATFORM_PITCH_ANGLE ),
      "Platform Pitch Angle (degrees)",
      "Aircraft pitch angle. Angle between longitudinal axis and horzontal "
      "plane. Positive angles above horizontal plane." },
    { TRAITS_OF( VITAL_META_PLATFORM_ROLL_ANGLE ),
      "Platform Roll Angle (degrees)",
      "Platform roll angle. Angle between transverse axis and horizontal "
      "plane. Positive angles for right wing lowered below horizontal plane." },
    { TRAITS_OF( VITAL_META_PLATFORM_TRUE_AIRSPEED ),
      "Platform True Airspeed (meters/second)",
      "True airspeed of platform. Indicated airspeed adjusted for temperature "
      "and altitude." },
    { TRAITS_OF( VITAL_META_PLATFORM_INDICATED_AIRSPEED ),
      "Platform Indicated Airspeed (meters/second)",
      "Indicated airspeed of platform. Derived from Pitot tube and static "
      "pressure sensors." },
    { TRAITS_OF( VITAL_META_PLATFORM_DESIGNATION ),
      "Platform Designation",
      "Platform designation." },
    { TRAITS_OF( VITAL_META_IMAGE_SOURCE_SENSOR ),
      "Image Source Sensor",
      "String of image source sensor.  E.g.: 'EO Nose', 'EO Zoom (DLTV)', "
      "'EO Spotter', 'IR Mitsubishi PtSi Model 500', 'IR InSb Amber Model "
      "TBT', 'LYNX SAR Imagery', 'TESAR Imagery', etc." },
    { TRAITS_OF( VITAL_META_IMAGE_COORDINATE_SYSTEM ),
      "Image Coordinate System",
      "Coordinate system used. E.g.: 'Geodetic WGS84', 'Geocentric WGS84', "
      "'TUM', 'None', etc." },
    { TRAITS_OF( VITAL_META_IMAGE_URI ),
      "Image URI",
      "URI of source image." },
    { TRAITS_OF( VITAL_META_VIDEO_DATA_STREAM_INDEX ),
      "Index of Metadata Stream",
      "Index of metadata stream." },
    { TRAITS_OF( VITAL_META_VIDEO_URI ),
      "Video URI",
      "URI of source video." },
    { TRAITS_OF( VITAL_META_VIDEO_KEY_FRAME ),
      "Is Key Frame",
      "True if the current frame is a key frame." },
    { TRAITS_OF( VITAL_META_VIDEO_FRAME_NUMBER ),
      "Frame Number",
      "Frame number of video input." },
    { TRAITS_OF( VITAL_META_VIDEO_MICROSECONDS ),
      "Video Relative Timestamp",
      "Microseconds since beginning of video input." },
    { TRAITS_OF( VITAL_META_SENSOR_LOCATION ),
      "Sensor Geodetic Location (lon/lat/alt)",
      "Three-dimensional coordinates of the sensor: longitude, latitude, and "
      "(optionally) altitude." },
    { TRAITS_OF( VITAL_META_SENSOR_HORIZONTAL_FOV ),
      "Sensor Horizonal Field of View (degrees)",
      "Horizontal field of view of selected imaging sensor." },
    { TRAITS_OF( VITAL_META_SENSOR_VERTICAL_FOV ),
      "Sensor Vertical Field of View (degrees)",
      "Vertical field of view of selected imaging sensor." },
    { TRAITS_OF( VITAL_META_SENSOR_REL_AZ_ANGLE ),
      "Sensor Relative Azimuth Angle (degrees)",
      "Relative rotation angle of sensor to platform longitudinal axis. "
      "Rotation angle between platform longitudinal axis and camera pointing "
      "direction as seen from above the platform." },
    { TRAITS_OF( VITAL_META_SENSOR_REL_EL_ANGLE ),
      "Sensor Relative Elevation Angle (degrees)",
      "Relative elevation Angle of sensor to platform longitudinal-transverse "
      "plane. Negative angles down." },
    { TRAITS_OF( VITAL_META_SENSOR_REL_ROLL_ANGLE ),
      "Sensor Relative Roll Angle (degrees)",
      "Relative roll angle of sensor to aircraft platform. Twisting angle of "
      "camera about lens axis. Top of image is zero degrees. Positive angles "
      "are clockwise when looking from behind camera." },
    { TRAITS_OF( VITAL_META_SENSOR_YAW_ANGLE ),
      "Sensor Yaw Angle (degrees)",
      "" },
    { TRAITS_OF( VITAL_META_SENSOR_PITCH_ANGLE ),
      "Sensor Pitch Angle (degrees)",
      "" },
    { TRAITS_OF( VITAL_META_SENSOR_ROLL_ANGLE ),
      "Sensor Roll Angle (degrees)",
      "" },
    { TRAITS_OF( VITAL_META_SENSOR_TYPE ),
      "Sensor Type",
      "" },
    { TRAITS_OF( VITAL_META_SLANT_RANGE ),
      "Slant Range (meters)",
      "Distance to target." },
    { TRAITS_OF( VITAL_META_TARGET_WIDTH ),
      "Target Width (meters)",
      "Target width within sensor field of view." },
    { TRAITS_OF( VITAL_META_FRAME_CENTER ),
      "Geodetic Frame Center (lon/lat/alt)",
      "Three-dimensional coordinates of the frame center: longitude, "
      "latitude, and (optionally) altitude." },
    { TRAITS_OF( VITAL_META_CORNER_POINTS ),
      "Corner Points (lon/lat)",
      "Four sided polygon representing the image bounds. The corners are "
      "ordered: upper left, upper right, lower right, lower left." },
    { TRAITS_OF( VITAL_META_ICING_DETECTED ),
      "Icing Detected",
      "Flag for icing detected at aircraft location." },
    { TRAITS_OF( VITAL_META_WIND_DIRECTION ),
      "Wind Direction (degrees)",
      "Wind direction at aircraft location. This is the direction the wind is "
      "coming from relative to true north." },
    { TRAITS_OF( VITAL_META_WIND_SPEED ),
      "Wind Speed (meters/second)",
      "Wind speed at aircraft location." },
    { TRAITS_OF( VITAL_META_STATIC_PRESSURE ),
      "Static Pressure (millibar)",
      "Static pressure at aircraft location." },
    { TRAITS_OF( VITAL_META_DENSITY_ALTITUDE ),
      "Density Altitude (meters)",
      "Density altitude at aircraft location. Relative aircraft performance "
      "metric based on outside air temperature, static pressure, and "
      "humidity." },
    { TRAITS_OF( VITAL_META_OUTSIDE_AIR_TEMPERATURE ),
      "Outside Air Temperature (Celsius)",
      "Temperature outside aircraft." },
    { TRAITS_OF( VITAL_META_TARGET_LOCATION ),
      "Target Geodetic Location (lon/lat/alt)",
      "Three-dimensional coordinates of the target: longitude, latitude, and "
      "(optionally) altitude." },
    { TRAITS_OF( VITAL_META_TARGET_TRK_GATE_WIDTH ),
      "Target Track Gate Width (pixels)",
      "Tracking gate width (x value) of tracked target within field of view." },
    { TRAITS_OF( VITAL_META_TARGET_TRK_GATE_HEIGHT ),
      "Target Track Gate Height (pixels)",
      "Tracking gate height (x value) of tracked target within field of view." },
    { TRAITS_OF( VITAL_META_TARGET_ERROR_EST_CE90 ),
      "Target Error Estimate - CE90 (meters)",
      "Circular Error 90 (CE90) is the estimated error distance in the "
      "horizontal direction. Specifies the radius of 90% probability on a "
      "plane tangent to the earth’s surface." },
    { TRAITS_OF( VITAL_META_TARGET_ERROR_EST_LE90 ),
      "Target Error Estimate - LE90 (meters)",
      "Lateral Error 90 (LE90) is the estimated error distance in the "
      "vertical (or lateral) direction. Specifies the interval of 90% "
      "probability in the local vertical direction." },
    { TRAITS_OF( VITAL_META_DIFFERENTIAL_PRESSURE ),
      "Differential Pressure (millibar)",
      "Differential pressure at aircraft location. Measured as the stagnation/"
      "impact/total pressure minus static pressure." },
    { TRAITS_OF( VITAL_META_PLATFORM_ANG_OF_ATTACK ),
      "Platform Angle of Attack (deg)",
      "Angle between platform longitudinal axis and relative wind. Positive "
      "angles for upward relative wind." },
    { TRAITS_OF( VITAL_META_PLATFORM_VERTICAL_SPEED ),
      "Platform Vertical Speed (meters/sec)",
      "Vertical speed of the aircraft relative to zenith. Positive ascending, "
      "negative descending." },
    { TRAITS_OF( VITAL_META_PLATFORM_SIDESLIP_ANGLE ),
      "Platform Sideslip Angle (degrees)",
      "The sideslip angle is the angle between the platform longitudinal axis "
      "and relative wind. Positive angles to right wing, negative to left." },
    { TRAITS_OF( VITAL_META_AIRFIELD_BAROMET_PRESS ),
      "Airfield Barometric Pressure (millibars)",
      "Local pressure at airfield of known height." },
    { TRAITS_OF( VITAL_META_AIRFIELD_ELEVATION ),
      "Airfield Elevation (meters)",
      "Elevation of airfield corresponding to Airfield Barometric Pressure." },
    { TRAITS_OF( VITAL_META_RELATIVE_HUMIDITY ),
      "Relative Humidity (percent)",
      "Relative humidity at aircraft location." },
    { TRAITS_OF( VITAL_META_PLATFORM_GROUND_SPEED ),
      "Platform Ground Speed (meters/second)",
      "Speed projected to the ground of an airborne platform passing "
      "overhead." },
    { TRAITS_OF( VITAL_META_GROUND_RANGE ),
      "Ground Range (meters)",
      "Horizontal distance from ground position of aircraft relative to nadir,"
      " and target of interest." },
    { TRAITS_OF( VITAL_META_PLATFORM_FUEL_REMAINING ),
      "Platform Fuel Remaining (kilograms)",
      "" },
    { TRAITS_OF( VITAL_META_PLATFORM_CALL_SIGN ),
      "Platform Call Sign",
      "Call sign of platform or operating unit." },
    { TRAITS_OF( VITAL_META_LASER_PRF_CODE ),
      "Laser PRF Code",
      "Pulse Repetition Frequency code used to mark a target. Three or four "
      "digit number consisting only of the digits 1..8." },
    { TRAITS_OF( VITAL_META_SENSOR_FOV_NAME ),
      "Sensor Field of View Name",
      "Names sensor field of view level in quantized steps: 4x Ultranarrow, "
      "2x Ultranarrow, Ultranarrow, Narrow, Narrow Medium, Medium, Wide, "
      "Ultrawide." },
    { TRAITS_OF( VITAL_META_PLATFORM_MAGNET_HEADING ),
      "Platform Magnetic Heading (degrees)",
      "Aircraft magnetic heading angle. Relative between longitudinal axis and"
      " Magnetic North measured in the horizontal plane." },
    { TRAITS_OF( VITAL_META_UAS_LDS_VERSION_NUMBER ),
      "UAS LDS Version Number",
      "" },
    { TRAITS_OF( VITAL_META_ANGLE_TO_NORTH ),
      "Angle to North (degrees)",
      "" },
    { TRAITS_OF( VITAL_META_OBLIQUITY_ANGLE ),
      "Sensor Obliquity Angle (degrees)",
      "" },
    { TRAITS_OF( VITAL_META_START_TIMESTAMP ),
      "Start Timestamp",
      "Time of collection start. Microseconds since UNIX epoch." },
    { TRAITS_OF( VITAL_META_EVENT_START_TIMESTAMP ),
      "Event Start Timestamp",
      "Time of event, mission, etc. start. Microseconds since UNIX epoch." },
    { TRAITS_OF( VITAL_META_SECURITY_CLASSIFICATION ),
      "Security Classification",
      "Security classification of source imagery." },
    { TRAITS_OF( VITAL_META_AVERAGE_GSD ),
      "Average Ground Sample Distance (meters/pixel)",
      "" },
    { TRAITS_OF( VITAL_META_VNIIRS ),
      "Video National Imagery Interpretability Rating Scale",
      "A subjective quality scale from 2 to 11 for rating the intelligence "
      "value of airborne motion imagery in the visible spectrum. See "
      "https://gwg.nga.mil/misb/docs/standards/ST0901.2.pdf" },
    { TRAITS_OF( VITAL_META_WAVELENGTH ),
      "Sensor Wavelength",
      "Wavelength band currently in use. Standardized values: 'VIS' "
      "(visible), 'IR' (infrared), 'NIR' (near/short-wave infrared), 'MIR' "
      "(mid-wave infrared), 'LIR' (long-wave infrared), 'FIR' "
      "(far-infrared)." },
    { TRAITS_OF( VITAL_META_GPS_SEC ),
      "GPS Seconds",
      "" },
    { TRAITS_OF( VITAL_META_GPS_WEEK ),
      "GPS Week",
      "" },
    { TRAITS_OF( VITAL_META_NORTHING_VEL ),
      "Northing Velocity (meters/second)",
      "" },
    { TRAITS_OF( VITAL_META_EASTING_VEL ),
      "Easting Velocity (meters/second)",
      "" },
    { TRAITS_OF( VITAL_META_UP_VEL ),
      "Up Velocity (meters/second)",
      "" },
    { TRAITS_OF( VITAL_META_IMU_STATUS ),
      "IMU Status",
      "" },
    { TRAITS_OF( VITAL_META_LOCAL_ADJ ),
      "Local Adj",
      "" },
    { TRAITS_OF( VITAL_META_DST_FLAGS ),
      "Dst Flags",
      "" },
    { TRAITS_OF( VITAL_META_RPC_HEIGHT_OFFSET ),
      "RPC Height Offset",
      "" },
    { TRAITS_OF( VITAL_META_RPC_HEIGHT_SCALE ),
      "RPC Height Scale",
      "" },
    { TRAITS_OF( VITAL_META_RPC_LONG_OFFSET ),
      "RPC Longitude Offset",
      "" },
    { TRAITS_OF( VITAL_META_RPC_LONG_SCALE ),
      "RPC Longitude Scale",
      "" },
    { TRAITS_OF( VITAL_META_RPC_LAT_OFFSET ),
      "RPC Latitude Offset",
      "" },
    { TRAITS_OF( VITAL_META_RPC_LAT_SCALE ),
      "RPC Latitude Scale",
      "" },
    { TRAITS_OF( VITAL_META_RPC_ROW_OFFSET ),
      "RPC Row Offset",
      "" },
    { TRAITS_OF( VITAL_META_RPC_ROW_SCALE ),
      "RPC Row Scale",
      "" },
    { TRAITS_OF( VITAL_META_RPC_COL_OFFSET ),
      "RPC Column Offset",
      "" },
    { TRAITS_OF( VITAL_META_RPC_COL_SCALE ),
      "RPC Column Scale",
      "" },
    { TRAITS_OF( VITAL_META_RPC_ROW_NUM_COEFF ),
      "RPC Row Numerator Coefficients",
      "" },
    { TRAITS_OF( VITAL_META_RPC_ROW_DEN_COEFF ),
      "RPC Row Denominator Coefficients",
      "" },
    { TRAITS_OF( VITAL_META_RPC_COL_NUM_COEFF ),
      "RPC Column Numerator Coefficients",
      "" },
    { TRAITS_OF( VITAL_META_RPC_COL_DEN_COEFF ),
      "RPC Column Denominator Coefficients",
      "" },
    { TRAITS_OF( VITAL_META_NITF_IDATIM ),
      "NITF IDATIM",
      "" },
    { TRAITS_OF( VITAL_META_NITF_BLOCKA_FRFC_LOC_01 ),
      "First Row First Column Location",
      "" },
    { TRAITS_OF( VITAL_META_NITF_BLOCKA_FRLC_LOC_01 ),
      "First Row Last Column Location",
      "" },
    { TRAITS_OF( VITAL_META_NITF_BLOCKA_LRFC_LOC_01 ),
      "Last Row First Column Location",
      "" },
    { TRAITS_OF( VITAL_META_NITF_BLOCKA_LRLC_LOC_01 ),
      "Last Row Last Column Location",
      "" },
    { TRAITS_OF( VITAL_META_NITF_IMAGE_COMMENTS ),
      "Image Comments for NITF File",
      "" }, };
  return array;
}

// ----------------------------------------------------------------------------
metadata_tag_traits const&
unknown_tag_traits()
{
  return tag_traits().at( 0 );
}

} // namespace

// ----------------------------------------------------------------------------
metadata_tag_traits
::metadata_tag_traits( vital_metadata_tag tag, std::string const& enum_name,
                       std::type_info const& type, std::string const& name,
                       std::string const& description )
  : m_tag{ tag }, m_enum_name{ enum_name }, m_type( type ), m_name{ name },
    m_description{ description }
{}

// ----------------------------------------------------------------------------
vital_metadata_tag
metadata_tag_traits
::tag() const
{
  return m_tag;
}

// ----------------------------------------------------------------------------
std::string
metadata_tag_traits
::name() const
{
  return m_name;
}

// ----------------------------------------------------------------------------
std::string
metadata_tag_traits
::enum_name() const
{
  return m_enum_name;
}

// ----------------------------------------------------------------------------
std::type_info const&
metadata_tag_traits
::type() const
{
  return m_type;
}

// ----------------------------------------------------------------------------
std::string
metadata_tag_traits
::type_name() const
{
  return demangle( m_type.name() );
}

// ----------------------------------------------------------------------------
std::string
metadata_tag_traits
::description() const
{
  return m_description;
}

// ----------------------------------------------------------------------------
metadata_tag_traits const&
tag_traits_by_tag( vital_metadata_tag tag )
{
  static auto const map =
    ( [](){
        std::map< vital_metadata_tag, metadata_tag_traits const* > result;
        for( auto const& traits : tag_traits() )
        {
          result.emplace( traits.tag(), &traits );
        }
        return result;
      } )();
  auto const it = map.find( tag );
  return ( it == map.end() ) ? unknown_tag_traits() : *it->second;
}

// ----------------------------------------------------------------------------
metadata_tag_traits const&
tag_traits_by_name( std::string const& name )
{
  static auto const map =
    ( [](){
        std::map< std::string, metadata_tag_traits const* > result;
        for( auto const& traits : tag_traits() )
        {
          result.emplace( traits.name(), &traits );
        }
        return result;
      } )();
  auto const it = map.find( name );
  return ( it == map.end() ) ? unknown_tag_traits() : *it->second;
}

// ----------------------------------------------------------------------------
metadata_tag_traits const&
tag_traits_by_enum_name( std::string const& name )
{
  static auto const map =
    ( [](){
        std::map< std::string, metadata_tag_traits const* > result;
        for( auto const& traits : tag_traits() )
        {
          result.emplace( traits.enum_name(), &traits );
        }
        return result;
      } )();
  auto const it = map.find( name );
  return ( it == map.end() ) ? unknown_tag_traits() : *it->second;
}

} // namespace vital

} // namespace kwiver
