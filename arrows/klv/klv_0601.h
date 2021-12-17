// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Interface to the KLV 0601 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_0601_H_
#define KWIVER_ARROWS_KLV_KLV_0601_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

#include "klv_0102.h"
#include "klv_set.h"
#include "klv_packet.h"

#include "vital/optional.h"

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0601_tag : klv_lds_key
{
  KLV_0601_UNKNOWN                             = 0,
  KLV_0601_CHECKSUM                            = 1,
  KLV_0601_PRECISION_TIMESTAMP                 = 2,
  KLV_0601_MISSION_ID                          = 3,
  KLV_0601_PLATFORM_TAIL_NUMBER                = 4,
  KLV_0601_PLATFORM_HEADING_ANGLE              = 5,
  KLV_0601_PLATFORM_PITCH_ANGLE                = 6,
  KLV_0601_PLATFORM_ROLL_ANGLE                 = 7,
  KLV_0601_PLATFORM_TRUE_AIRSPEED              = 8,
  KLV_0601_PLATFORM_INDICATED_AIRSPEED         = 9,
  KLV_0601_PLATFORM_DESIGNATION                = 10,
  KLV_0601_IMAGE_SOURCE_SENSOR                 = 11,
  KLV_0601_IMAGE_COORDINATE_SYSTEM             = 12,
  KLV_0601_SENSOR_LATITUDE                     = 13,
  KLV_0601_SENSOR_LONGITUDE                    = 14,
  KLV_0601_SENSOR_TRUE_ALTITUDE                = 15,
  KLV_0601_SENSOR_HORIZONTAL_FOV               = 16,
  KLV_0601_SENSOR_VERTICAL_FOV                 = 17,
  KLV_0601_SENSOR_RELATIVE_AZIMUTH_ANGLE       = 18,
  KLV_0601_SENSOR_RELATIVE_ELEVATION_ANGLE     = 19,
  KLV_0601_SENSOR_RELATIVE_ROLL_ANGLE          = 20,
  KLV_0601_SLANT_RANGE                         = 21,
  KLV_0601_TARGET_WIDTH                        = 22,
  KLV_0601_FRAME_CENTER_LATITUDE               = 23,
  KLV_0601_FRAME_CENTER_LONGITUDE              = 24,
  KLV_0601_FRAME_CENTER_ELEVATION              = 25,
  KLV_0601_OFFSET_CORNER_LATITUDE_POINT_1      = 26,
  KLV_0601_OFFSET_CORNER_LONGITUDE_POINT_1     = 27,
  KLV_0601_OFFSET_CORNER_LATITUDE_POINT_2      = 28,
  KLV_0601_OFFSET_CORNER_LONGITUDE_POINT_2     = 29,
  KLV_0601_OFFSET_CORNER_LATITUDE_POINT_3      = 30,
  KLV_0601_OFFSET_CORNER_LONGITUDE_POINT_3     = 31,
  KLV_0601_OFFSET_CORNER_LATITUDE_POINT_4      = 32,
  KLV_0601_OFFSET_CORNER_LONGITUDE_POINT_4     = 33,
  KLV_0601_ICING_DETECTED                      = 34,
  KLV_0601_WIND_DIRECTION                      = 35,
  KLV_0601_WIND_SPEED                          = 36,
  KLV_0601_STATIC_PRESSURE                     = 37,
  KLV_0601_DENSITY_ALTITUDE                    = 38,
  KLV_0601_OUTSIDE_AIR_TEMPERATURE             = 39,
  KLV_0601_TARGET_LOCATION_LATITUDE            = 40,
  KLV_0601_TARGET_LOCATION_LONGITUDE           = 41,
  KLV_0601_TARGET_LOCATION_ELEVATION           = 42,
  KLV_0601_TARGET_TRACK_GATE_WIDTH             = 43,
  KLV_0601_TARGET_TRACK_GATE_HEIGHT            = 44,
  KLV_0601_TARGET_ERROR_ESTIMATE_CE90          = 45,
  KLV_0601_TARGET_ERROR_ESTIMATE_LE90          = 46,
  KLV_0601_GENERIC_FLAG_DATA                   = 47,
  KLV_0601_SECURITY_LOCAL_SET                  = 48,
  KLV_0601_DIFFERENTIAL_PRESSURE               = 49,
  KLV_0601_PLATFORM_ANGLE_OF_ATTACK            = 50,
  KLV_0601_PLATFORM_VERTICAL_SPEED             = 51,
  KLV_0601_PLATFORM_SIDESLIP_ANGLE             = 52,
  KLV_0601_AIRFIELD_BAROMETRIC_PRESSURE        = 53,
  KLV_0601_AIRFIELD_ELEVATION                  = 54,
  KLV_0601_RELATIVE_HUMIDITY                   = 55,
  KLV_0601_PLATFORM_GROUND_SPEED               = 56,
  KLV_0601_GROUND_RANGE                        = 57,
  KLV_0601_PLATFORM_FUEL_REMAINING             = 58,
  KLV_0601_PLATFORM_CALL_SIGN                  = 59,
  KLV_0601_WEAPON_LOAD                         = 60,
  KLV_0601_WEAPON_FIRED                        = 61,
  KLV_0601_LASER_PRF_CODE                      = 62,
  KLV_0601_SENSOR_FOV_NAME                     = 63,
  KLV_0601_PLATFORM_MAGNETIC_HEADING           = 64,
  KLV_0601_VERSION_NUMBER                      = 65,
  KLV_0601_DEPRECATED                          = 66,
  KLV_0601_ALTERNATE_PLATFORM_LATITUDE         = 67,
  KLV_0601_ALTERNATE_PLATFORM_LONGITUDE        = 68,
  KLV_0601_ALTERNATE_PLATFORM_ALTITUDE         = 69,
  KLV_0601_ALTERNATE_PLATFORM_NAME             = 70,
  KLV_0601_ALTERNATE_PLATFORM_HEADING          = 71,
  KLV_0601_EVENT_START_TIME                    = 72,
  KLV_0601_RVT_LOCAL_SET                       = 73,
  KLV_0601_VMTI_LOCAL_SET                      = 74,
  KLV_0601_SENSOR_ELLIPSOID_HEIGHT             = 75,
  KLV_0601_ALTERNATE_PLATFORM_ELLIPSOID_HEIGHT = 76,
  KLV_0601_OPERATIONAL_MODE                    = 77,
  KLV_0601_FRAME_CENTER_HEIGHT_ABOVE_ELLIPSOID = 78,
  KLV_0601_SENSOR_NORTH_VELOCITY               = 79,
  KLV_0601_SENSOR_EAST_VELOCITY                = 80,
  KLV_0601_IMAGE_HORIZON_PIXEL_PACK            = 81,
  KLV_0601_FULL_CORNER_LATITUDE_POINT_1        = 82,
  KLV_0601_FULL_CORNER_LONGITUDE_POINT_1       = 83,
  KLV_0601_FULL_CORNER_LATITUDE_POINT_2        = 84,
  KLV_0601_FULL_CORNER_LONGITUDE_POINT_2       = 85,
  KLV_0601_FULL_CORNER_LATITUDE_POINT_3        = 86,
  KLV_0601_FULL_CORNER_LONGITUDE_POINT_3       = 87,
  KLV_0601_FULL_CORNER_LATITUDE_POINT_4        = 88,
  KLV_0601_FULL_CORNER_LONGITUDE_POINT_4       = 89,
  KLV_0601_FULL_PLATFORM_PITCH_ANGLE           = 90,
  KLV_0601_FULL_PLATFORM_ROLL_ANGLE            = 91,
  KLV_0601_FULL_PLATFORM_ANGLE_OF_ATTACK       = 92,
  KLV_0601_FULL_PLATFORM_SIDESLIP_ANGLE        = 93,
  KLV_0601_MIIS_CORE_IDENTIFIER                = 94,
  KLV_0601_SAR_MOTION_IMAGERY_LOCAL_SET        = 95,
  KLV_0601_TARGET_WIDTH_EXTENDED               = 96,
  KLV_0601_RANGE_IMAGE_LOCAL_SET               = 97,
  KLV_0601_GEOREGISTRATION_LOCAL_SET           = 98,
  KLV_0601_COMPOSITE_IMAGING_LOCAL_SET         = 99,
  KLV_0601_SEGMENT_LOCAL_SET                   = 100,
  KLV_0601_AMEND_LOCAL_SET                     = 101,
  KLV_0601_SDCC_FLP                            = 102,
  KLV_0601_DENSITY_ALTITUDE_EXTENDED           = 103,
  KLV_0601_SENSOR_ELLIPSOID_HEIGHT_EXTENDED    = 104,
  KLV_0601_ALTERNATE_PLATFORM_ELLIPSOID_HEIGHT_EXTENDED
                                               = 105,
  KLV_0601_STREAM_DESIGNATOR                   = 106,
  KLV_0601_OPERATIONAL_BASE                    = 107,
  KLV_0601_BROADCAST_SOURCE                    = 108,
  KLV_0601_RANGE_TO_RECOVERY_LOCATION          = 109,
  KLV_0601_TIME_AIRBORNE                       = 110,
  KLV_0601_PROPULSION_UNIT_SPEED               = 111,
  KLV_0601_PLATFORM_COURSE_ANGLE               = 112,
  KLV_0601_ALTITUDE_ABOVE_GROUND_LEVEL         = 113,
  KLV_0601_RADAR_ALTIMETER                     = 114,
  KLV_0601_CONTROL_COMMAND                     = 115,
  KLV_0601_CONTROL_COMMAND_VERIFICATION_LIST   = 116,
  KLV_0601_SENSOR_AZIMUTH_RATE                 = 117,
  KLV_0601_SENSOR_ELEVATION_RATE               = 118,
  KLV_0601_SENSOR_ROLL_RATE                    = 119,
  KLV_0601_ONBOARD_MI_STORAGE_PERCENT_FULL     = 120,
  KLV_0601_ACTIVE_WAVELENGTH_LIST              = 121,
  KLV_0601_COUNTRY_CODES                       = 122,
  KLV_0601_NUMBER_OF_NAVSATS_IN_VIEW           = 123,
  KLV_0601_POSITIONING_METHOD_SOURCE           = 124,
  KLV_0601_PLATFORM_STATUS                     = 125,
  KLV_0601_SENSOR_CONTROL_MODE                 = 126,
  KLV_0601_SENSOR_FRAME_RATE_PACK              = 127,
  KLV_0601_WAVELENGTHS_LIST                    = 128,
  KLV_0601_TARGET_ID                           = 129,
  KLV_0601_AIRBASE_LOCATIONS                   = 130,
  KLV_0601_TAKEOFF_TIME                        = 131,
  KLV_0601_TRANSMISSION_FREQUENCY              = 132,
  KLV_0601_ONBOARD_MI_STORAGE_CAPACITY         = 133,
  KLV_0601_ZOOM_PERCENTAGE                     = 134,
  KLV_0601_COMMUNICATIONS_METHOD               = 135,
  KLV_0601_LEAP_SECONDS                        = 136,
  KLV_0601_CORRECTION_OFFSET                   = 137,
  KLV_0601_PAYLOAD_LIST                        = 138,
  KLV_0601_ACTIVE_PAYLOADS                     = 139,
  KLV_0601_WEAPONS_STORES                      = 140,
  KLV_0601_WAYPOINT_LIST                       = 141,
  KLV_0601_VIEW_DOMAIN                         = 142,
  KLV_0601_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0601_tag tag );

// ----------------------------------------------------------------------------
/// Indicates whether ice forming on the aircraft has been detected.
enum KWIVER_ALGO_KLV_EXPORT klv_0601_icing_detected
{
  KLV_0601_ICING_DETECTED_DETECTOR_OFF,
  KLV_0601_ICING_DETECTED_FALSE,
  KLV_0601_ICING_DETECTED_TRUE,
  KLV_0601_ICING_DETECTED_ENUM_END,
};

// ----------------------------------------------------------------------------
/// Interprets data as a ST0601 icing detection status.
using klv_0601_icing_detected_format =
  klv_enum_format< klv_0601_icing_detected >;

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0601_icing_detected value );

// ----------------------------------------------------------------------------
/// Indicates one of several discrete zoom levels.
enum KWIVER_ALGO_KLV_EXPORT klv_0601_sensor_fov_name
{
  KLV_0601_SENSOR_FOV_NAME_ULTRANARROW,
  KLV_0601_SENSOR_FOV_NAME_NARROW,
  KLV_0601_SENSOR_FOV_NAME_MEDIUM,
  KLV_0601_SENSOR_FOV_NAME_WIDE,
  KLV_0601_SENSOR_FOV_NAME_ULTRAWIDE,
  KLV_0601_SENSOR_FOV_NAME_NARROW_MEDIUM,
  KLV_0601_SENSOR_FOV_NAME_2X_ULTRANARROW,
  KLV_0601_SENSOR_FOV_NAME_4X_ULTRANARROW,
  KLV_0601_SENSOR_FOV_NAME_CONTINUOUS_ZOOM,
  KLV_0601_SENSOR_FOV_NAME_ENUM_END,
};

// ----------------------------------------------------------------------------
/// Interprets data as a ST0601 discrete sensor field of view.
using klv_0601_sensor_fov_name_format =
  klv_enum_format< klv_0601_sensor_fov_name >;

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0601_sensor_fov_name value );

// ----------------------------------------------------------------------------
/// Miscellaneous boolean values
enum KWIVER_ALGO_KLV_EXPORT klv_0601_generic_flag_data_bits
{
  // 0 = laser off, 1 = laser on
  KLV_0601_GENERIC_FLAG_DATA_BIT_LASER_RANGE,
  // 0 = auto-track off, 1 = auto-track on
  KLV_0601_GENERIC_FLAG_DATA_BIT_AUTO_TRACK,
  // 0 = black hot, 1 = white hot
  KLV_0601_GENERIC_FLAG_DATA_BIT_IR_POLARITY,
  // 0 = no icing detected, 1 = icing detected
  KLV_0601_GENERIC_FLAG_DATA_BIT_ICING_STATUS,
  // 0 = slant range calculated, 1 = slant range measured
  KLV_0601_GENERIC_FLAG_DATA_BIT_SLANT_RANGE,
  // 0 = image valid, 1 = image invalid
  KLV_0601_GENERIC_FLAG_DATA_BIT_IMAGE_INVALID,
  KLV_0601_GENERIC_FLAG_DATA_BIT_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0601_generic_flag_data_bits value );

// ----------------------------------------------------------------------------
/// Indicates the operational mode of the FMV-producing platform.
enum KWIVER_ALGO_KLV_EXPORT klv_0601_operational_mode
{
  KLV_0601_OPERATIONAL_MODE_OTHER,
  KLV_0601_OPERATIONAL_MODE_OPERATIONAL,
  KLV_0601_OPERATIONAL_MODE_TRAINING,
  KLV_0601_OPERATIONAL_MODE_EXERCISE,
  KLV_0601_OPERATIONAL_MODE_MAINTENANCE,
  KLV_0601_OPERATIONAL_MODE_TEST,
  KLV_0601_OPERATIONAL_MODE_ENUM_END,
};

// ----------------------------------------------------------------------------
/// Interprets data as a ST0601 UAV operational mode.
using klv_0601_operational_mode_format =
  klv_enum_format< klv_0601_operational_mode >;

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0601_operational_mode value );

// ----------------------------------------------------------------------------
/// Indicates the general status of the aircraft.
enum KWIVER_ALGO_KLV_EXPORT klv_0601_platform_status
{
  KLV_0601_PLATFORM_STATUS_ACTIVE,
  KLV_0601_PLATFORM_STATUS_PREFLIGHT,
  KLV_0601_PLATFORM_STATUS_PREFLIGHT_TAXIING,
  KLV_0601_PLATFORM_STATUS_RUNUP,
  KLV_0601_PLATFORM_STATUS_TAKEOFF,
  KLV_0601_PLATFORM_STATUS_INGRESS,
  KLV_0601_PLATFORM_STATUS_MANUAL_OPERATION,
  KLV_0601_PLATFORM_STATUS_AUTOMATED_ORBIT,
  KLV_0601_PLATFORM_STATUS_TRANSITIONING,
  KLV_0601_PLATFORM_STATUS_EGRESS,
  KLV_0601_PLATFORM_STATUS_LANDING,
  KLV_0601_PLATFORM_STATUS_LANDING_TAXIING,
  KLV_0601_PLATFORM_STATUS_LANDED_PARKED,
  KLV_0601_PLATFORM_STATUS_ENUM_END,
};

// ----------------------------------------------------------------------------
/// Interprets data as a ST0601 UAV platform status.
using klv_0601_platform_status_format =
  klv_enum_format< klv_0601_platform_status >;

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0601_platform_status value );

// ----------------------------------------------------------------------------
/// Indicates how the sensor is being operated.
enum KWIVER_ALGO_KLV_EXPORT klv_0601_sensor_control_mode
{
  KLV_0601_SENSOR_CONTROL_MODE_OFF,
  KLV_0601_SENSOR_CONTROL_MODE_HOME_POSITION,
  KLV_0601_SENSOR_CONTROL_MODE_UNCONTROLLED,
  KLV_0601_SENSOR_CONTROL_MODE_MANUAL_CONTROL,
  KLV_0601_SENSOR_CONTROL_MODE_CALIBRATING,
  KLV_0601_SENSOR_CONTROL_MODE_AUTO_HOLDING_POSITION,
  KLV_0601_SENSOR_CONTROL_MODE_AUTO_TRACKING,
  KLV_0601_SENSOR_CONTROL_MODE_ENUM_END,
};

// ----------------------------------------------------------------------------
/// Interprets data as a ST0601 sensor control mode.
using klv_0601_sensor_control_mode_format =
  klv_enum_format< klv_0601_sensor_control_mode >;

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0601_sensor_control_mode value );

// ----------------------------------------------------------------------------
/// Record of command sent to UAV.
struct KWIVER_ALGO_KLV_EXPORT klv_0601_control_command
{
  uint16_t id;
  std::string string;
  uint64_t timestamp;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0601_control_command const& value );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator==( klv_0601_control_command const& lhs,
            klv_0601_control_command const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator<( klv_0601_control_command const& lhs,
           klv_0601_control_command const& rhs );

// ----------------------------------------------------------------------------
/// Interprets data as a ST0601 control command.
class KWIVER_ALGO_KLV_EXPORT klv_0601_control_command_format
  : public klv_data_format_< klv_0601_control_command >
{
public:
  klv_0601_control_command_format();

  std::string
  description() const override;

private:
  klv_0601_control_command
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_0601_control_command const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_0601_control_command const& value,
                   size_t length_hint ) const override;
};

// ----------------------------------------------------------------------------
/// Frame rate expressed as a ratio of integers.
struct KWIVER_ALGO_KLV_EXPORT klv_0601_frame_rate
{
  uint32_t numerator;
  uint32_t denominator;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0601_frame_rate const& value );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator==( klv_0601_frame_rate const& lhs, klv_0601_frame_rate const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator<( klv_0601_frame_rate const& lhs, klv_0601_frame_rate const& rhs );

// ----------------------------------------------------------------------------
/// Interprets data as a frame rate.
class KWIVER_ALGO_KLV_EXPORT klv_0601_frame_rate_format
  : public klv_data_format_< klv_0601_frame_rate >
{
public:
  klv_0601_frame_rate_format();

  std::string
  description() const override;

private:
  klv_0601_frame_rate
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_0601_frame_rate const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_0601_frame_rate const& value,
                   size_t length_hint ) const override;
};

// ----------------------------------------------------------------------------
/// Record of involvement of various countries in production of the FMV.
struct klv_0601_country_codes
{
  klv_0102_country_coding_method coding_method;
  kwiver::vital::optional< std::string > overflight_country;
  kwiver::vital::optional< std::string > operator_country;
  kwiver::vital::optional< std::string > country_of_manufacture;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0601_country_codes const& value );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator==( klv_0601_country_codes const& lhs,
            klv_0601_country_codes const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator<( klv_0601_country_codes const& lhs,
           klv_0601_country_codes const& rhs );

// ----------------------------------------------------------------------------
/// Interprets data as country codes.
class KWIVER_ALGO_KLV_EXPORT klv_0601_country_codes_format
  : public klv_data_format_< klv_0601_country_codes >
{
public:
  klv_0601_country_codes_format();

  std::string
  description() const override;

private:
  klv_0601_country_codes
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_0601_country_codes const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_0601_country_codes const& value,
                   size_t length_hint ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as a MISB ST0601 local set.
class KWIVER_ALGO_KLV_EXPORT klv_0601_local_set_format
  : public klv_local_set_format
{
public:
  klv_0601_local_set_format();

  std::string
  description() const override;

private:
  klv_local_set
  read_typed( klv_read_iter_t& data, size_t length ) const override;


  uint16_t
  calculate_checksum( klv_read_iter_t data, size_t length ) const override;

  uint16_t
  read_checksum( klv_read_iter_t data, size_t length ) const override;

  void
  write_checksum( uint16_t checksum,
                  klv_write_iter_t& data, size_t max_length ) const override;

  size_t
  checksum_length() const override;
};

// ----------------------------------------------------------------------------
/// Returns the UDS key for a MISB ST0601 local set.
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_0601_key();

// ----------------------------------------------------------------------------
/// Returns a lookup object for the traits of the ST0601 tags.
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0601_traits_lookup();

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
