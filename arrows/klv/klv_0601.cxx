// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "klv_0601.h"

#include "klv_0806.h"
#include "klv_0903.h"
#include "klv_1002.h"
#include "klv_1010.h"
#include "klv_1204.h"
#include "klv_1206.h"
#include "klv_1601.h"
#include "klv_1602.h"
#include "klv_1607.h"
#include "klv_checksum.h"
#include "klv_series.hpp"
#include "klv_length_value.h"
#include "klv_list.hpp"
#include "klv_util.h"

#include <vital/logger/logger.h>

#include <algorithm>
#include <iomanip>
#include <set>
#include <vector>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0601_traits_lookup()
{
  // Constants here are taken from Section 8 of
  // https://gwg.nga.mil/misb/docs/standards/ST0601.17.pdf
  // Descriptions are edited for clarity, brevity, consistency, etc.
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0601_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown Tag",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0601_CHECKSUM ),
      std::make_shared< klv_uint_format >( 2 ),
      "Checksum",
      "Checksum used to detect errors within a ST 0601 packet.",
      0 },
    { // No universal key for members of ST0601
      {},
      // KWIVER enum value
      ENUM_AND_NAME( KLV_0601_PRECISION_TIMESTAMP ),
      // KLV Format: uint64
      std::make_shared< klv_uint_format >( 8 ),
      // Item name
      "Precision Timestamp",
      // Item description
      "Timestamp for all metadata in a ST 0601 local set. Used to coordinate "
      "with Motion Imagery.",
      // Exactly one tag allowed (mandatory)
      1 },
    { {},
      ENUM_AND_NAME( KLV_0601_MISSION_ID ),
      std::make_shared< klv_string_format >(),
      "Mission ID",
      "Descriptive mission identifier to distinguish an event or sortie.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_TAIL_NUMBER ),
      std::make_shared< klv_string_format >(),
      "Platform Tail Number",
      "Identifier of platform as posted.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_HEADING_ANGLE ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 360.0 }, 2 ),
      "Platform Heading Angle",
      "Angle between longitudinal axis and true north measured in the "
      "horizontal plane. Angles increase in a clockwise direction when "
      "looking from above the platform. Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_PITCH_ANGLE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -20.0, 20.0 }, 2 ),
      "Platform Pitch Angle",
      "Angle between longitudinal axis and horizonal plane. Positive angles "
      "above horizonal plane. Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_ROLL_ANGLE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -50.0, 50.0 }, 2 ),
      "Platform Roll Angle",
      "Angle between transverse axis and transverse-longitudinal plane. "
      "Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_TRUE_AIRSPEED ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 255.0 }, 1 ),
      "Platform True Airspeed",
      "True airspeed of the platform: indicated airspeed adjusted for "
      "temperature and altitude. Measured in meters per second.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_INDICATED_AIRSPEED ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 255.0 }, 1 ),
      "Platform Indicated Airspeed",
      "Indicated airspeed of the platform. Derived from Pitot tube and static "
      "pressure sensors. Measured in meters per second.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_DESIGNATION ),
      std::make_shared< klv_string_format >(),
      "Platform Designation",
      "Model name for the platform. Examples: 'Predator', 'Reaper'.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_IMAGE_SOURCE_SENSOR ),
      std::make_shared< klv_string_format >(),
      "Image Source Sensor",
      "Name of the currently active sensor. Examples: 'EO Nose', 'TESAR "
      "Imagery'.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_IMAGE_COORDINATE_SYSTEM ),
      std::make_shared< klv_string_format >(),
      "Image Coordinate System",
      "Name of the image coordinate system used.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_LATITUDE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -90.0, 90.0 }, 4 ),
      "Sensor Latitude",
      "Latitude of the currently active sensor, based on WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_LONGITUDE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -180.0, 180.0 }, 4 ),
      "Sensor Longitude",
      "Longitude of the currently active sensor, based on WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_TRUE_ALTITUDE ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ -900.0, 19000.0 }, 2 ),
      "Sensor True Altitude",
      "Altitude of the currently active sensor, relative to Mean Sea Level. "
      "Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_HORIZONTAL_FOV ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 180.0 }, 2 ),
      "Sensor Horizontal Field of View",
      "Horizonal field of view of the currently active sensor. Measured in "
      "degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_VERTICAL_FOV ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 180.0 }, 2 ),
      "Sensor Vertical Field of View",
      "Vertical field of view of the currently active sensor. Measured in "
      "degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_RELATIVE_AZIMUTH_ANGLE ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 360.0 }, 4 ),
      "Sensor Relative Azimuth Angle",
      "Relative rotation angle of the currently active sensor to the platform "
      "longitudinal axis. Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_RELATIVE_ELEVATION_ANGLE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -180.0, 180.0 }, 4 ),
      "Sensor Relative Elevation Angle",
      "Relative elevation angle of the currently active sensor to the "
      "platform longitudinal-transverse plane. Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_RELATIVE_ROLL_ANGLE ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 360.0 }, 4 ),
      "Sensor Relative Roll Angle",
      "Relative roll angle of the currently active sensor to the platform. "
      "Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SLANT_RANGE ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 5.0e6 }, 4 ),
      "Slant Range",
      "Distance between currently active sensor and the image center. "
      "Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_TARGET_WIDTH ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 1.0e4 }, 2 ),
      "Target Width",
      "Target width within sensor field of view. Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FRAME_CENTER_LATITUDE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -90.0, 90.0 }, 4 ),
      "Frame Center Latitude",
      "Latitude of image center, based on WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FRAME_CENTER_LONGITUDE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -180.0, 180.0 }, 4 ),
      "Frame Center Longitude",
      "Longitude of image center, based on WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FRAME_CENTER_ELEVATION ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ -900.0, 19000.0 }, 2 ),
      "Frame Center Elevation",
      "Altitude of image center, relative to Mean Sea Level.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_OFFSET_CORNER_LATITUDE_POINT_1 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -0.075, 0.075 }, 2 ),
      "Offset Corner Latitude Point 1",
      "Latitude offset for the upper left corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_OFFSET_CORNER_LONGITUDE_POINT_1 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -0.075, 0.075 }, 2 ),
      "Offset Corner Longitude Point 1",
      "Longitude offset for the upper left corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_OFFSET_CORNER_LATITUDE_POINT_2 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -0.075, 0.075 }, 2 ),
      "Offset Corner Latitude Point 2",
      "Latitude offset for the upper right corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_OFFSET_CORNER_LONGITUDE_POINT_2 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -0.075, 0.075 }, 2 ),
      "Offset Corner Longitude Point 2",
      "Longitude offset for the upper right corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_OFFSET_CORNER_LATITUDE_POINT_3 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -0.075, 0.075 }, 2 ),
      "Offset Corner Latitude Point 3",
      "Latitude offset for the lower right corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_OFFSET_CORNER_LONGITUDE_POINT_3 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -0.075, 0.075 }, 2 ),
      "Offset Corner Longitude Point 3",
      "Longitude offset for the lower right corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_OFFSET_CORNER_LATITUDE_POINT_4 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -0.075, 0.075 }, 2 ),
      "Offset Corner Latitude Point 4",
      "Latitude offset for the lower left corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_OFFSET_CORNER_LONGITUDE_POINT_4 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -0.075, 0.075 }, 2 ),
      "Offset Corner Longitude Point 4",
      "Longitude offset for the lower left corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_ICING_DETECTED ),
      std::make_shared< klv_enum_format< klv_0601_icing_detected > >( 1 ),
      "Icing Detected",
      "Flag for whether icing is detected on the aircraft.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_WIND_DIRECTION ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 360.0 }, 2 ),
      "Wind Direction",
      "Wind direction at the aircraft's location relative to true north. "
      "Angle increases in a clockwise direction when looking from above the "
      "platform. Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_WIND_SPEED ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 100.0 }, 1 ),
      "Wind Speed",
      "Wind speed at the aircraft's location. Measured in meters per second.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_STATIC_PRESSURE ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 5000.0 }, 2 ),
      "Static Pressure",
      "Static pressure at the aircraft's location. Measured in millibar.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_DENSITY_ALTITUDE ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ -900.0, 19000.0 }, 2 ),
      "Density Altitude",
      "Density altitude at the aircraft's location. Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_OUTSIDE_AIR_TEMPERATURE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -127.0, 127.0 }, 1 ),
      "Outside Air Temperature",
      "Temperature outside of the aircraft. Measured in degrees Celsius.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_TARGET_LOCATION_LATITUDE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -90.0, 90.0 }, 4 ),
      "Target Location Latitude",
      "Calculated target latitude, based on WGS84 ellipsoid. Tracks "
      "crosshair, not image center.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_TARGET_LOCATION_LONGITUDE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -180.0, 180.0 }, 4 ),
      "Target Location Longitude",
      "Calculated target longitude, based on WGS84 ellipsoid. Tracks "
      "crosshair, not image center.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_TARGET_LOCATION_ELEVATION ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ -900.0, 19000.0 }, 2 ),
      "Target Location Elevation",
      "Calculated target altitude, relative to Mean Sea Level. Tracks "
      "crosshair, not image center. Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_TARGET_TRACK_GATE_WIDTH ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 510.0 }, 1 ),
      "Target Track Gate Width",
      "Width of box around tracked target. Measured in pixels.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_TARGET_TRACK_GATE_HEIGHT ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 510.0 }, 1 ),
      "Target Track Gate Height",
      "Height of box around tracked target. Measured in pixels.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_TARGET_ERROR_ESTIMATE_CE90 ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 4095.0 }, 2 ),
      "Target Error Estimate - CE90",
      "Radius of 90% confidence for the target location in the horizonal "
      "direction. Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_TARGET_ERROR_ESTIMATE_LE90 ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 4095.0 }, 2 ),
      "Target Error Estimate - LE90",
      "Radius of 90% confidence for the target location in the vertical "
      "direction. Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_GENERIC_FLAG_DATA ),
      std::make_shared< klv_0601_generic_flag_data_format >( 1 ),
      "Generic Flag Data",
      "Bits representing miscellaneous boolean values.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SECURITY_LOCAL_SET ),
      std::make_shared< klv_0102_local_set_format >(),
      "Security Local Set",
      "MISB ST 0102 local set for security metadata.",
      { 0, 1 },
      &klv_0102_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0601_DIFFERENTIAL_PRESSURE ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 5000.0 }, 2 ),
      "Differential Pressure",
      "Differential pressure at the aircraft's location. Equal to total "
      "pressure minus static pressure. Measured in millibar.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_ANGLE_OF_ATTACK ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -20.0, 20.0 }, 2 ),
      "Platform Angle of Attack",
      "Vertical angle between the platform longitudinal axis and the relative "
      "wind. Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_VERTICAL_SPEED ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -180.0, 180.0 }, 2 ),
      "Platform Vertical Speed",
      "Vertical speed of aircraft in the zenith direction. Measured in meters "
      "per second.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_SIDESLIP_ANGLE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -20.0, 20.0 }, 2 ),
      "Platform Sideslip Angle",
      "Horizontal angle between the platform longitudinal axis and the "
      "relative wind. Angle increases in a clockwise direction when looking "
      "from above the platform. Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_AIRFIELD_BAROMETRIC_PRESSURE ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 5000.0 }, 2 ),
      "Airfield Barometric Pressure",
      "Local pressure at the airfield. Measured in millibar.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_AIRFIELD_ELEVATION ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ -900.0, 19000.0 }, 2 ),
      "Airfield Elevation",
      "Altitude of the airfield, relative to Mean Sea Level. Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_RELATIVE_HUMIDITY ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 100.0 }, 1 ),
      "Relative Humidity",
      "Relative humidity at the aircraft location. Ratio between the current "
      "water vapor density and the saturation point of water vapor density. "
      "Measured in percent.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_GROUND_SPEED ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 255.0 }, 1 ),
      "Platform Ground Speed",
      "Speed of the aircraft when projected onto the ground plane.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_GROUND_RANGE ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 5.0e6 }, 4 ),
      "Ground Range",
      "Horizontal distance between the aircraft and the target of interest.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_FUEL_REMAINING ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 1.0e4 }, 2 ),
      "Platform Fuel Remaining",
      "Current weight of fuel present on the aircraft. Measured in kilograms.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_CALL_SIGN ),
      std::make_shared< klv_string_format >(),
      "Platform Call Sign",
      "Call sign of the platform or operating unit.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_WEAPON_LOAD ),
      std::make_shared< klv_uint_format >( 2 ),
      "Weapon Load",
      "Current weapons stored on aircraft. Deprecated by Item 140 (Weapon "
      "Stores).",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_WEAPON_FIRED ),
      std::make_shared< klv_uint_format >( 1 ),
      "Weapon Fired",
      "Signal when a particular weapon is released.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_LASER_PRF_CODE ),
      std::make_shared< klv_uint_format >( 2 ),
      "Laser PRF Code",
      "A laser's pulse repetition frequency used to mark a target. Three or "
      "four digit number with digits 1-8.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_FOV_NAME ),
      std::make_shared< klv_0601_sensor_fov_name_format >( 1  ),
      "Sensor Field of View Name",
      "Current lens type.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_MAGNETIC_HEADING ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 360.0 }, 2 ),
      "Platform Magnetic Heading",
      "Angle between longitudingal axis and Magnetic North measured in the "
      "horizontal plane. Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_VERSION_NUMBER ),
      std::make_shared< klv_uint_format >( 1 ),
      "UAS Datalink LS Version Number",
      "Major version of MISB ST 0601 used as the source standard when "
      "encoding this local set.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0601_DEPRECATED ),
      std::make_shared< klv_blob_format >(),
      "Deprecated Tag",
      "This item is deprecated.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0601_ALTERNATE_PLATFORM_LATITUDE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -90.0, 90.0 }, 4 ),
      "Alternate Platform Latitude",
      "Latitude of the platform connected to the UAS via direct datalink.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_ALTERNATE_PLATFORM_LONGITUDE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -180.0, 180.0 }, 4 ),
      "Alternate Platform Longitude",
      "Longitude of the platform connected to the UAS via direct datalink.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_ALTERNATE_PLATFORM_ALTITUDE ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ -900.0, 19000.0 }, 2 ),
      "Alternate Platform Altitude",
      "Altitude of the platform connected to the UAS via direct datalink.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_ALTERNATE_PLATFORM_NAME ),
      std::make_shared< klv_string_format >(),
      "Alternate Platform Name",
      "Name of the platform connected to the UAS via direct datalink. "
      "Examples: 'Apache', 'Rover'.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_ALTERNATE_PLATFORM_HEADING ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ 0.0, 360.0 }, 2 ),
      "Alternate Platform Heading",
      "Heading angle of the platform connected to the UAS via direct datalink.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_EVENT_START_TIME ),
      std::make_shared< klv_uint_format >( 8 ),
      "Event Start Time",
      "Start time of scene, project, event, mission, editing event, license, "
      "publication, etc, represented in the number of UTC microseconds "
      "elapsed since midnight, January 1, 1970.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_RVT_LOCAL_SET ),
      std::make_shared< klv_0806_local_set_format >(),
      "RVT Local Set",
      "MISB ST 0806 local set for remote video terminals.",
      { 0, 1 },
      &klv_0806_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0601_VMTI_LOCAL_SET ),
      std::make_shared< klv_0903_local_set_format >(),
      "VMTI Local Set",
      "MISB ST 0903 local set for the video moving target indicator.",
      { 0, 1 },
      &klv_0903_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_ELLIPSOID_HEIGHT ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ -900.0, 19000.0 }, 2 ),
      "Sensor Ellipsoid Height",
      "Altitude of the currently active sensor, relative to the WGS84 "
      "ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_ALTERNATE_PLATFORM_ELLIPSOID_HEIGHT ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ -900.0, 19000.0 }, 2 ),
      "Alternate Platform Ellipsoid Height",
      "Altitude of the platform connected to the UAS via direct datalink, "
      "relative to the WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_OPERATIONAL_MODE ),
      std::make_shared< klv_0601_operational_mode_format >( 1 ),
      "Operational Mode",
      "Mode of operation of the event portrayed in the Motion Imagery.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FRAME_CENTER_ELLIPSOID_HEIGHT ),
      std::make_shared< klv_uflint_format >(
        kv::interval< double >{ -900.0, 19000.0 }, 2 ),
      "Frame Center Height Above Ellipsoid",
      "Altitude of frame center, relative to the WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_NORTH_VELOCITY ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -327.0, 327.0 }, 2 ),
      "Sensor North Velocity",
      "Northing velocity of the sensor or platform.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_EAST_VELOCITY ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -327.0, 327.0 }, 2 ),
      "Sensor East Velocity",
      "Easting velocity of the sensor or platform.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_IMAGE_HORIZON_PIXEL_PACK ),
      std::make_shared< klv_0601_image_horizon_pixel_pack_format >(),
      "Image Horizon Pixel Pack",
      "Location of earth-sky horizon in the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FULL_CORNER_LATITUDE_POINT_1 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -90.0, 90.0 }, 4 ),
      "Corner Latitude Point 1 (Full)",
      "Latitude for the upper left corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FULL_CORNER_LONGITUDE_POINT_1 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -180.0, 180.0 }, 4 ),
      "Corner Longitude Point 1 (Full)",
      "Longitude for the upper left corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FULL_CORNER_LATITUDE_POINT_2 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -90.0, 90.0 }, 4 ),
      "Corner Latitude Point 2 (Full)",
      "Latitude for the upper right corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FULL_CORNER_LONGITUDE_POINT_2 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -180.0, 180.0 }, 4 ),
      "Corner Longitude Point 2 (Full)",
      "Longitude for the upper right corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FULL_CORNER_LATITUDE_POINT_3 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -90.0, 90.0 }, 4 ),
      "Corner Latitude Point 3 (Full)",
      "Latitude for the lower right corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FULL_CORNER_LONGITUDE_POINT_3 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -180.0, 180.0 }, 4 ),
      "Corner Longitude Point 3 (Full)",
      "Longitude for the lower right corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FULL_CORNER_LATITUDE_POINT_4 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -90.0, 90.0 }, 4 ),
      "Corner Latitude Point 4 (Full)",
      "Latitude for the lower left corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FULL_CORNER_LONGITUDE_POINT_4 ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -180.0, 180.0 }, 4 ),
      "Corner Longitude Point 4 (Full)",
      "Longitude for the lower left corner of the image.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FULL_PLATFORM_PITCH_ANGLE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -90.0, 90.0 }, 4 ),
      "Platform Pitch Angle (Full)",
      "Angle between longitudinal axis and horizonal plane. Positive angles "
      "above horizonal plane. Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FULL_PLATFORM_ROLL_ANGLE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -90.0, 90.0 }, 4 ),
      "Platform Roll Angle (Full)",
      "Angle between transverse axis and transverse-longitudinal plane. "
      "Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FULL_PLATFORM_ANGLE_OF_ATTACK ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -90.0, 90.0 }, 4 ),
      "Platform Angle of Attack (Full)",
      "Vertical angle between the platform longitudinal axis and the relative "
      "wind. Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_FULL_PLATFORM_SIDESLIP_ANGLE ),
      std::make_shared< klv_sflint_format >(
        kv::interval< double >{ -180.0, 180.0 }, 4 ),
      "Platform Sideslip Angle (Full)",
      "Horizontal angle between the platform longitudinal axis and the "
      "relative wind. Angle increases in a clockwise direction when looking from above the platform. Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_MIIS_CORE_IDENTIFIER ),
      std::make_shared< klv_1204_miis_id_format >(),
      "MIIS Core Identifier",
      "Binary value of MISB ST 1201 core identifier.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SAR_MOTION_IMAGERY_LOCAL_SET ),
      std::make_shared< klv_1206_local_set_format >(),
      "SAR Motion Imagery Local Set",
      "MISB ST 1206 local set for synthetic aperture radar Motion Imagery.",
      { 0, 1 },
      &klv_1206_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0601_TARGET_WIDTH_EXTENDED ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ 0.0, 1.5e6 } ),
      "Target Width Extended",
      "Target width within sensor field of view. Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_RANGE_IMAGE_LOCAL_SET ),
      std::make_shared< klv_1002_local_set_format >(),
      "Range Image Local Set",
      "MISB ST 1002 local set for range images.",
      { 0, 1 },
      &klv_1002_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0601_GEOREGISTRATION_LOCAL_SET ),
      std::make_shared< klv_1601_local_set_format >(),
      "Geo-Registration Local Set",
      "MISB ST 1601 local set for geo-registration.",
      { 0, 1 },
      &klv_1601_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0601_COMPOSITE_IMAGING_LOCAL_SET ),
      std::make_shared< klv_1602_local_set_format >(),
      "Composite Imaging Local Set",
      "MISB ST 1602 local set for composite imaging.",
      { 0, 1 },
      &klv_1602_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0601_SEGMENT_LOCAL_SET ),
      std::make_shared< klv_1607_child_set_format >( lookup ),
      "Segment Local Set",
      "MISB ST 1607 Segment local set for metadata sharing across parent and "
      "child sets.",
      { 0, SIZE_MAX } },
    { {},
      ENUM_AND_NAME( KLV_0601_AMEND_LOCAL_SET ),
      std::make_shared< klv_1607_child_set_format >( lookup ),
      "Amend Local Set",
      "MISB ST 1607 Amend local set for metadata corrections.",
      { 0, SIZE_MAX } },
    { {},
      ENUM_AND_NAME( KLV_0601_SDCC_FLP ),
      std::make_shared< klv_1010_sdcc_flp_format >(),
      "SDCC-FLP",
      "MISB ST 101 floating length pack for standard deviation and "
      "cross-correlation metadata.",
      { 0, SIZE_MAX } },
    { {},
      ENUM_AND_NAME( KLV_0601_DENSITY_ALTITUDE_EXTENDED ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ -900.0, 40000.0 } ),
      "Density Altitude Extended",
      "Density altitude at the aircraft's location. Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_ELLIPSOID_HEIGHT_EXTENDED ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ -900.0, 40000.0 } ),
      "Sensor Ellipsoid Height Extended",
      "Altitude of the currently active sensor, relative to the WGS84 "
      "ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_ALTERNATE_PLATFORM_ELLIPSOID_HEIGHT_EXTENDED ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ -900.0, 40000.0 } ),
      "Alternate Platform Ellipsoid Height Extended",
      "Altitude of the platform connected to the UAS via direct datalink, "
      "relative to the WGS84 ellipsoid.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_STREAM_DESIGNATOR ),
      std::make_shared< klv_string_format >(),
      "Stream Designator",
      "Shorthand descriptor for a particular Motion Imagery data stream, "
      "typically delivered over IP.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_OPERATIONAL_BASE ),
      std::make_shared< klv_string_format >(),
      "Operational Base",
      "Indicates the location for the launch recovery equipment.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_BROADCAST_SOURCE ),
      std::make_shared< klv_string_format >(),
      "Broadcast Source",
      "Location where the Motion Imagery is first broadcast. Examples: "
      "'Creech', 'Cannon'.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_RANGE_TO_RECOVERY_LOCATION ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ 0.0, 21000.0 } ),
      "Range to Recovery Location",
      "Distance from current position to airframe recovery position. Measured "
      "in kilometers.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_TIME_AIRBORNE ),
      std::make_shared< klv_uint_format >(),
      "Time Airborne",
      "Number of seconds the aircraft has been airborne.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PROPULSION_UNIT_SPEED ),
      std::make_shared< klv_uint_format >(),
      "Propulsion Unit Speed",
      "Speed at which the engine or motor is rotating.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_COURSE_ANGLE ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ 0.0, 360.0 } ),
      "Platform Course Angle",
      "Angle between aircraft velocity vector and true north measured in the "
      "horizontal plane. Angles increase in a clockwise direction when "
      "looking from above the platform. Measured in degrees.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_ALTITUDE_ABOVE_GROUND_LEVEL ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ -900.0, 40000.0 } ),
      "Altitude Above Ground Level",
      "Vertical distance between the aircraft and the ground or water. "
      "Measured in meters.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_RADAR_ALTIMETER ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ -900.0, 40000.0 } ),
      "Radar Altimeter",
      "Vertical distance between the aircraft and the ground or water, as "
      "measured by a radar altimeter.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_CONTROL_COMMAND ),
      std::make_shared< klv_0601_control_command_format >(),
      "Control Command",
      "Record of command from ground control station to aircraft.",
      { 0, SIZE_MAX } },
    { {},
      ENUM_AND_NAME( KLV_0601_CONTROL_COMMAND_VERIFICATION_LIST ),
      std::make_shared< klv_0601_control_command_verify_list_format >(),
      "Control Command Verification List",
      "Acknowledgement from the platform that one or more control commands "
      "were received.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_AZIMUTH_RATE ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ -1000.0, 1000.0 } ),
      "Sensor Azimuth Rate",
      "Rate at which the sensor is rotating clockwise, when looking down from "
      "above the aircraft. Measured in degrees per second.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_ELEVATION_RATE ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ -1000.0, 1000.0 } ),
      "Sensor Elevation Rate",
      "Rate at which the sensor is rotating clockwise, when looking at the "
      "aircraft from the side such that the aircraft is pointing left. "
      "Measured in degrees per second.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_ROLL_RATE ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ -1000.0, 1000.0 } ),
      "Sensor Roll Rate",
      "Rate at which the sensor is rotating clockwise, when looking from "
      "behind the sensor. Measured in degrees per second.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_ONBOARD_MI_STORAGE_PERCENT_FULL ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ 0.0, 100.0 } ),
      "On-board MI Storage Percent Full",
      "Amount of on-board Motion Imagery storage used as a percentage of "
      "total storage.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_ACTIVE_WAVELENGTH_LIST ),
      std::make_shared< klv_0601_active_wavelength_list_format >(),
      "Active Wavelength List",
      "List of wavelengths used by the sensor to generate the Motion Imagery.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_COUNTRY_CODES ),
      std::make_shared< klv_0601_country_codes_format >(),
      "Country Codes",
      "Countries which are associated with the platform and its operation.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_NUMBER_OF_NAVSATS_IN_VIEW ),
      std::make_shared< klv_uint_format >( 1 ),
      "Number of NAVSATs in View",
      "Number of satellites used to determine position.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_POSITIONING_METHOD_SOURCE ),
      std::make_shared< klv_0601_positioning_method_source_format >( 1 ),
      "Positioning Method Source",
      "Source of the navigation positioning information.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PLATFORM_STATUS ),
      std::make_shared< klv_0601_platform_status_format >(),
      "Platform Status",
      "Operational mode of the platform.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_CONTROL_MODE ),
      std::make_shared< klv_0601_sensor_control_mode_format >(),
      "Sensor Control Mode",
      "Sensor control operational status.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_SENSOR_FRAME_RATE_PACK ),
      std::make_shared< klv_0601_frame_rate_format >(),
      "Sensor Frame Rate Pack",
      "Frame rate of the Motion Imagery at the sensor.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_WAVELENGTHS_LIST ),
      std::make_shared< klv_0601_wavelengths_list_format >(),
      "Wavelengths List",
      "List of wavelength bands provided by all available sensors.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_TARGET_ID ),
      std::make_shared< klv_string_format >(),
      "Target ID",
      "Alpha-numeric identification of the target.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_AIRBASE_LOCATIONS ),
      std::make_shared< klv_0601_airbase_locations_format >(),
      "Airbase Locations",
      "Geographic location of take-off and recovery site.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_TAKEOFF_TIME ),
      std::make_shared< klv_uint_format >(),
      "Take-off Time",
      "Time when aircraft became airborne.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_TRANSMISSION_FREQUENCY ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ 1.0, 99999.0 } ),
      "Transmission Frequency",
      "Radio frequency used to transmit the Motion Imagery. Measured in "
      "megahertz.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_ONBOARD_MI_STORAGE_CAPACITY ),
      std::make_shared< klv_uint_format >(),
      "On-board MI Storage Capacity",
      "Total capacity of on-board Motion Imagery storage. Measured in "
      "gigabytes.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_ZOOM_PERCENTAGE ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ 0.0, 100.0 } ),
      "Zoom Percentage",
      "For a variable zoom system, the current percentage of zoom.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_COMMUNICATIONS_METHOD ),
      std::make_shared< klv_string_format >(),
      "Communications Method",
      "Type of communications used with platform",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_LEAP_SECONDS ),
      std::make_shared< klv_sint_format >(),
      "Leap Seconds",
      "Number of leap seconds to adjust Precision Timestamp (Item 2) to UTC.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_CORRECTION_OFFSET ),
      std::make_shared< klv_sint_format >(),
      "Correction Offset",
      "Post-flight time adjustment for Precision Timestamp (Item 2) as "
      "needed.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_PAYLOAD_LIST ),
      std::make_shared< klv_0601_payload_list_format >(),
      "Payload List",
      "List of payloads available on platform.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_ACTIVE_PAYLOADS ),
      std::make_shared< klv_0601_active_payloads_format >(),
      "Active Payloads",
      "List of currently active payloads from the payload list (Item 138).",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_WEAPONS_STORES ),
      std::make_shared< klv_0601_weapons_store_list_format >(),
      "Weapons Stores",
      "List of weapon stores and statuses.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_WAYPOINT_LIST ),
      std::make_shared< klv_0601_waypoint_list_format >(),
      "Waypoint List",
      "List of navigational waypoints and their statuses.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0601_VIEW_DOMAIN ),
      std::make_shared< klv_0601_view_domain_format >(),
      "View Domain",
      "Specifies range of possible sensor relative azimuth, elevation, and "
      "roll values.",
      { 0, 1 } }, };

  return lookup;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_icing_detected value )
{
  static std::string strings[ KLV_0601_ICING_DETECTED_ENUM_END + 1 ] = {
    "Detector Off",
    "No Icing Detected",
    "Icing Detected",
    "Unknown Icing Detection Status" };

  return os << strings[ std::min( value, KLV_0601_ICING_DETECTED_ENUM_END ) ];
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_sensor_fov_name value )
{
  static std::string strings[ KLV_0601_SENSOR_FOV_NAME_ENUM_END + 1 ] = {
    "Ultranarrow",
    "Narrow",
    "Medium",
    "Wide",
    "Ultrawide",
    "Narrow Medium",
    "2x Ultranarrow",
    "4x Ultranarrow",
    "Continuous Zoom",
    "Unknown Sensor FOV Name" };

  os << strings[ std::min( value, KLV_0601_SENSOR_FOV_NAME_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_positioning_method_source_bit value )
{
  static std::string strings[ KLV_0601_POSITIONING_METHOD_SOURCE_BIT_ENUM_END +
                              1 ] = {
    "On-board INS",
    "GPS",
    "Galileo",
    "QZSS",
    "NAVIC",
    "GLONASS",
    "BeiDou-1",
    "BeiDou-2",
    "Unknown Positioning Method Source Bit" };

  os <<
    strings[ std::min( value,
                       KLV_0601_POSITIONING_METHOD_SOURCE_BIT_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_generic_flag_data_bit value )
{
  static std::string strings[ KLV_0601_GENERIC_FLAG_DATA_BIT_ENUM_END + 1 ] = {
    "Laser Range",
    "Auto-Track",
    "IR Polarity",
    "Icing Status",
    "Slant Range",
    "Image Invalid",
    "Unknown Generic Flag Data Bit" };

  os << strings[ std::min( value, KLV_0601_GENERIC_FLAG_DATA_BIT_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0601_operational_mode value )
{
  static std::string strings[ KLV_0601_OPERATIONAL_MODE_ENUM_END + 1 ] = {
    "Other",
    "Operational",
    "Training",
    "Exercise",
    "Maintenance",
    "Test",
    "Unknown Operational Mode" };

  os << strings[ std::min( value, KLV_0601_OPERATIONAL_MODE_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0601_platform_status value )
{
  static std::string strings[ KLV_0601_PLATFORM_STATUS_ENUM_END + 1 ] = {
    "Active",
    "Preflight",
    "Preflight - Taxiing",
    "Run-up",
    "Take-off",
    "Ingress",
    "Manual Operation",
    "Automated Orbit",
    "Transitioning",
    "Egress",
    "Landing",
    "Landing - Taxiing",
    "Landed - Parked",
    "Unknown Platform Status" };

  return os << strings[ std::min( value, KLV_0601_PLATFORM_STATUS_ENUM_END ) ];
}

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0601_sensor_control_mode value )
{
  static std::string strings[ KLV_0601_SENSOR_CONTROL_MODE_ENUM_END + 1 ] = {
    "Off",
    "Home Position",
    "Uncontrolled",
    "Manual Control",
    "Calibrating",
    "Auto - Holding Position",
    "Auto - Tracking",
    "Unknown Sensor Control Mode" };

  os << strings[ std::min( value, KLV_0601_SENSOR_CONTROL_MODE_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_image_horizon_locations const& value )
{
  os << "{ "
     << "location0: { "
     << "latitude: " << value.latitude0 << ", "
     << "longitude: " << value.longitude0 << " }, "
     << "location1: { "
     << "latitude: " << value.latitude1 << ", "
     << "longitude: " << value.longitude1 << " } }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0601_image_horizon_locations,
  &klv_0601_image_horizon_locations::latitude0,
  &klv_0601_image_horizon_locations::longitude0,
  &klv_0601_image_horizon_locations::latitude1,
  &klv_0601_image_horizon_locations::longitude1
)

// ----------------------------------------------------------------------------
klv_0601_image_horizon_locations_format
::klv_0601_image_horizon_locations_format()
  : klv_data_format_< klv_0601_image_horizon_locations >{ 16 }
{}

// ----------------------------------------------------------------------------
std::string
klv_0601_image_horizon_locations_format
::description() const
{
  return "image horizon locations of " + length_description();
}

// ----------------------------------------------------------------------------
klv_0601_image_horizon_locations
klv_0601_image_horizon_locations_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_0601_image_horizon_locations result;
  result.latitude0 =
    klv_read_flint< uint32_t >( { -90.0, 90.0 }, data, tracker.verify( 4 ) );
  result.longitude0 =
    klv_read_flint< uint32_t >( { -180.0, 180.0 }, data, tracker.verify( 4 ) );
  result.latitude1 =
    klv_read_flint< uint32_t >( { -90.0, 90.0 }, data, tracker.verify( 4 ) );
  result.longitude1 =
    klv_read_flint< uint32_t >( { -180.0, 180.0 }, data, tracker.verify( 4 ) );
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0601_image_horizon_locations_format
::write_typed( klv_0601_image_horizon_locations const& value,
              klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_write_flint< uint32_t >( value.latitude0, { -90.0, 90.0 },
                               data, tracker.verify( 4 ) );
  klv_write_flint< uint32_t >( value.longitude0, { -180.0, 180.0 },
                               data, tracker.verify( 4 ) );
  klv_write_flint< uint32_t >( value.latitude1, { -90.0, 90.0 },
                               data, tracker.verify( 4 ) );
  klv_write_flint< uint32_t >( value.longitude1, { -180.0, 180.0 },
                               data, tracker.verify( 4 ) );
}

// ----------------------------------------------------------------------------
size_t
klv_0601_image_horizon_locations_format
::length_of_typed(
  VITAL_UNUSED klv_0601_image_horizon_locations const& value ) const
{
  return 16;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_image_horizon_pixel_pack const& value )
{
  os << "{ "
     << "point0: { "
     << static_cast< unsigned int >( value.x0 ) << ", "
     << static_cast< unsigned int >( value.y0 ) << " }, "
     << "point1: { "
     << static_cast< unsigned int >( value.x1 ) << ", "
     << static_cast< unsigned int >( value.y1 ) << " } }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0601_image_horizon_pixel_pack,
  &klv_0601_image_horizon_pixel_pack::x0,
  &klv_0601_image_horizon_pixel_pack::y0,
  &klv_0601_image_horizon_pixel_pack::x1,
  &klv_0601_image_horizon_pixel_pack::y1,
  &klv_0601_image_horizon_pixel_pack::locations
)

// ----------------------------------------------------------------------------
klv_0601_image_horizon_pixel_pack_format
::klv_0601_image_horizon_pixel_pack_format()
  : klv_data_format_< klv_0601_image_horizon_pixel_pack >{ 0 }
{}

// ----------------------------------------------------------------------------
std::string
klv_0601_image_horizon_pixel_pack_format
::description() const
{
  return "image horizon pixel pack of " + length_description();
}

// ----------------------------------------------------------------------------
klv_0601_image_horizon_pixel_pack
klv_0601_image_horizon_pixel_pack_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_0601_image_horizon_pixel_pack result;
  result.x0 = klv_read_int< uint8_t >( data, tracker.verify( 1 ) );
  result.y0 = klv_read_int< uint8_t >( data, tracker.verify( 1 ) );
  result.x1 = klv_read_int< uint8_t >( data, tracker.verify( 1 ) );
  result.y1 = klv_read_int< uint8_t >( data, tracker.verify( 1 ) );
  if( tracker.remaining() )
  {
    klv_0601_image_horizon_locations_format format;
    result.locations = format.read_( data, tracker.verify( 16 ) );
  }
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0601_image_horizon_pixel_pack_format
::write_typed( klv_0601_image_horizon_pixel_pack const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_write_int( value.x0, data, tracker.verify( 1 ) );
  klv_write_int( value.y0, data, tracker.verify( 1 ) );
  klv_write_int( value.x1, data, tracker.verify( 1 ) );
  klv_write_int( value.y1, data, tracker.verify( 1 ) );
  if( value.locations )
  {
    klv_0601_image_horizon_locations_format format;
    format.write_( *value.locations, data, tracker.verify( 16 ) );
  }
}

// ----------------------------------------------------------------------------
size_t
klv_0601_image_horizon_pixel_pack_format
::length_of_typed( klv_0601_image_horizon_pixel_pack const& value ) const
{
  return 4 + ( value.locations ? 16 : 0 );
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_control_command const& value )
{
  return os << "{ " << "ID: " << value.id
            << ", String: \"" << value.string
            << "\", Timestamp: " << value.timestamp
            << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0601_control_command,
  &klv_0601_control_command::id,
  &klv_0601_control_command::string,
  &klv_0601_control_command::timestamp
)

// ----------------------------------------------------------------------------
klv_0601_control_command_format
::klv_0601_control_command_format() : klv_data_format_< data_type >{ 0 }
{}

// ----------------------------------------------------------------------------
klv_0601_control_command
klv_0601_control_command_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  klv_0601_control_command result;
  auto const tracker = track_it( data, length );
  result.id = klv_read_ber_oid< uint16_t >( data, tracker.remaining() );

  auto const length_of_string =
    klv_read_ber< size_t >( data, tracker.remaining() );
  if( length_of_string > tracker.remaining() )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "reading command string overruns data buffer" );
  }

  result.string = klv_read_string( data, length_of_string );
  if( tracker.remaining() )
  {
    result.timestamp = klv_read_int< uint64_t >( data, tracker.verify( 8 ) );
  }
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0601_control_command_format
::write_typed( klv_0601_control_command const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_write_ber_oid( value.id, data, tracker.remaining() );

  auto const length_of_string = klv_string_length( value.string );
  klv_write_ber( length_of_string, data, tracker.remaining() );
  klv_write_string( value.string, data, tracker.remaining() );
  if( value.timestamp )
  {
    klv_write_int( *value.timestamp, data, tracker.verify( 8 ) );
  }
}

// ----------------------------------------------------------------------------
size_t
klv_0601_control_command_format
::length_of_typed( klv_0601_control_command const& value ) const
{
  return klv_ber_oid_length( value.id ) +
         klv_ber_length( klv_string_length( value.string ) ) +
         klv_string_length( value.string ) +
         ( value.timestamp ? 8 : 0 );
}

// ----------------------------------------------------------------------------
std::string
klv_0601_control_command_format
::description() const
{
  return "control command of " + length_description();
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_frame_rate const& value )
{
  if( value.denominator == 1 )
  {
    return os << value.numerator;
  }
  if( value.denominator == 0 )
  {
    return os << "(invalid)";
  }
  return os << std::fixed << std::setprecision( 3 )
            << ( static_cast< double >( value.numerator ) /
                 value.denominator );
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0601_frame_rate,
  &klv_0601_frame_rate::numerator,
  &klv_0601_frame_rate::denominator
)

// ----------------------------------------------------------------------------
klv_0601_frame_rate_format
::klv_0601_frame_rate_format() : klv_data_format_< data_type >{ 0 } {}

// ----------------------------------------------------------------------------
klv_0601_frame_rate
klv_0601_frame_rate_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  klv_0601_frame_rate result;
  auto const tracker = track_it( data, length );
  result.numerator = klv_read_ber_oid< uint32_t >( data, tracker.remaining() );
  result.denominator =
    tracker.remaining()
    ? klv_read_ber_oid< uint32_t >( data, tracker.remaining() ) : 1;
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0601_frame_rate_format
::write_typed( klv_0601_frame_rate const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_write_ber_oid( value.numerator, data, tracker.remaining() );
  if( value.denominator != 1 )
  {
    klv_write_ber_oid( value.denominator, data, tracker.remaining() );
  }
}

// ----------------------------------------------------------------------------
size_t
klv_0601_frame_rate_format
::length_of_typed( klv_0601_frame_rate const& value ) const
{
  return klv_ber_oid_length( value.numerator ) +
         ( ( value.denominator == 1 )
           ? 0 : klv_ber_oid_length( value.denominator ) );
}

// ----------------------------------------------------------------------------
std::string
klv_0601_frame_rate_format
::description() const
{
  return "frame rate of " + length_description();
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_country_codes const& value )
{
  return
    os << "{ "
       << "coding method: "
       << value.coding_method
       << ", "
       << "overflight country: "
       << ( value.overflight_country
        ? *value.overflight_country
        : "(empty)" )
       << ", "
       << "operator country: "
       << ( value.operator_country
        ? *value.operator_country
        : "(empty)" )
       << ", "
       << "country of manufacture: "
       << ( value.country_of_manufacture
        ? *value.country_of_manufacture
        : "(empty)" )
       << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0601_country_codes,
  &klv_0601_country_codes::coding_method,
  &klv_0601_country_codes::overflight_country,
  &klv_0601_country_codes::operator_country,
  &klv_0601_country_codes::country_of_manufacture
)

// ----------------------------------------------------------------------------
klv_0601_country_codes_format
::klv_0601_country_codes_format()
  : klv_data_format_< klv_0601_country_codes >{ 0 }
{}

// ----------------------------------------------------------------------------
std::string
klv_0601_country_codes_format
::description() const
{
  return "country codes pack of " + length_description();
}

// ----------------------------------------------------------------------------
klv_0601_country_codes
klv_0601_country_codes_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  klv_0601_country_codes result = {};
  auto const tracker = track_it( data, length );

  // Read coding method
  auto const length_of_coding_method =
    klv_read_ber< size_t >( data, tracker.remaining() );
  result.coding_method =
    static_cast< klv_0102_country_coding_method >(
      klv_read_int< uint64_t >(
        data, tracker.verify( length_of_coding_method ) ) );

  // Read overflight country
  auto const length_of_overflight_country =
    klv_read_ber< size_t >( data, tracker.remaining() );
  if( length_of_overflight_country )
  {
    result.overflight_country =
      klv_read_string( data, tracker.verify( length_of_overflight_country ) );
  }

  // Read operator country
  if( !tracker.remaining() )
  {
    // The last two country codes have been omitted
    return result;
  }

  auto const length_of_operator_country =
    klv_read_ber< size_t >( data, tracker.remaining() );
  if( length_of_operator_country )
  {
    result.operator_country =
      klv_read_string( data, tracker.verify( length_of_operator_country ) );
  }

  // Read country of manufacture
  if( !tracker.remaining() )
  {
    // The last country code has been omitted
    return result;
  }

  auto const length_of_country_of_manufacture =
    klv_read_ber< size_t >( data, tracker.remaining() );
  if( length_of_country_of_manufacture )
  {
    result.country_of_manufacture =
      klv_read_string(
        data, tracker.verify( length_of_country_of_manufacture ) );
  }

  return result;
}

// ----------------------------------------------------------------------------
void
klv_0601_country_codes_format
::write_typed( klv_0601_country_codes const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );

  // Write coding method
  size_t const length_of_coding_method = 1;
  klv_write_ber( length_of_coding_method, data, tracker.remaining() );
  klv_write_int( static_cast< size_t >( value.coding_method ), data,
                 tracker.verify( length_of_coding_method ) );

  // Write overflight country
  if( value.overflight_country )
  {
    auto const field_length = klv_string_length( *value.overflight_country );
    klv_write_ber< size_t >( field_length, data, tracker.remaining() );
    klv_write_string( *value.overflight_country, data, tracker.remaining() );
  }
  else
  {
    klv_write_ber< size_t >( 0, data, tracker.remaining() );
  }

  // Write operator country
  if( value.operator_country )
  {
    auto const field_length = klv_string_length( *value.operator_country );
    klv_write_ber< size_t >( field_length, data, tracker.remaining() );
    klv_write_string( *value.operator_country, data, tracker.remaining() );
  }
  else if( value.country_of_manufacture )
  {
    // Cannot omit if the next field is not omitted
    klv_write_ber< size_t >( 0, data, tracker.remaining() );
  }
  else
  {
    // Omit this and the next field
    return;
  }

  // Write country of manufacture
  if( value.country_of_manufacture )
  {
    auto const field_length =
      klv_string_length( *value.country_of_manufacture );
    klv_write_ber< size_t >( field_length, data, tracker.remaining() );
    klv_write_string( *value.country_of_manufacture, data,
                      tracker.remaining() );
  }
  // Omit (write nothing) for final field if no value is present
}

// ----------------------------------------------------------------------------
size_t
klv_0601_country_codes_format
::length_of_typed( klv_0601_country_codes const& value ) const
{
  // Cannot be omitted
  size_t const length_of_coding_method = 1;
  size_t const length_of_length_of_coding_method = 1;

  // Cannot be omitted
  auto const length_of_overflight_country =
    value.overflight_country
    ? klv_string_length( *value.overflight_country )
    : 0;
  auto const length_of_length_of_overflight_country =
    klv_ber_length( length_of_overflight_country );

  // Can be omitted if this field has no value
  auto const length_of_country_of_manufacture =
    value.country_of_manufacture
    ? klv_string_length( *value.country_of_manufacture )
    : 0;
  auto const length_of_length_of_country_of_manufacture =
    length_of_country_of_manufacture
    ? klv_ber_length( length_of_country_of_manufacture )
    : 0;

  // Can be omitted if this field and country of manufacture each have no value
  auto const length_of_operator_country =
    value.operator_country
    ? klv_string_length( *value.operator_country )
    : 0;
  auto const length_of_length_of_operator_country =
    ( length_of_country_of_manufacture || length_of_operator_country )
    ? klv_ber_length( length_of_operator_country )
    : 0;

  return length_of_length_of_coding_method +
         length_of_coding_method +
         length_of_length_of_overflight_country +
         length_of_overflight_country +
         length_of_length_of_operator_country +
         length_of_operator_country +
         length_of_length_of_country_of_manufacture +
         length_of_country_of_manufacture;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_location_dlp const& value )
{
  return
    os << "{ "
       << "latitude: "
       << value.latitude
       << ", "
       << "longitude: "
       << value.longitude
       << ", "
       << "altitude: "
       << ( value.altitude
        ? std::to_string( *value.altitude )
        : std::string( "(empty)" ) )
       << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0601_location_dlp,
  &klv_0601_location_dlp::latitude,
  &klv_0601_location_dlp::longitude,
  &klv_0601_location_dlp::altitude
)

// ----------------------------------------------------------------------------
klv_0601_location_dlp_format
::klv_0601_location_dlp_format()
  : klv_data_format_< klv_0601_location_dlp >{ 0 }
{}

// ----------------------------------------------------------------------------
std::string
klv_0601_location_dlp_format
::description() const
{
  return "location pack of " + length_description();
}

// ----------------------------------------------------------------------------
klv_0601_location_dlp
klv_0601_location_dlp_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  klv_0601_location_dlp result = {};
  auto const tracker = track_it( data, length );

  result.latitude =
    klv_read_imap( { -90.0, 90.0 }, data, tracker.verify( 4 ) );
  result.longitude =
    klv_read_imap( { -180.0, 180.0 }, data, tracker.verify( 4 ) );
  // Altitude is not required
  if( tracker.remaining() )
  {
    // Altitude is set
    result.altitude =
      klv_read_imap( { -900.0, 9000.0 }, data, tracker.verify( 3 ) );
  }

  return result;
}

// ----------------------------------------------------------------------------
void
klv_0601_location_dlp_format
::write_typed( klv_0601_location_dlp const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );

  klv_write_imap( value.latitude,
                  { -90.0, 90.0 },
                  data, tracker.verify( 4 ) );
  klv_write_imap( value.longitude,
                  { -180.0, 180.0 },
                  data, tracker.verify( 4 ) );
  if( value.altitude )
  {
    klv_write_imap( *value.altitude,
                    { -900.0, 9000.0 },
                    data, tracker.verify( 3 ) );
  }
}

// ----------------------------------------------------------------------------
size_t
klv_0601_location_dlp_format
::length_of_typed( klv_0601_location_dlp const& value ) const
{
  // Latitude (4) and longitude (4) are required, altitude (3) is optional
  return 8 + ( value.altitude ? 3 : 0 );
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_airbase_locations const& value )
{
  os << "{ "
     << "take-off: "
     << *value.take_off_location
     << ", "
     << "recovery: ";
  if( value.recovery_location )
  {
    os << *value.recovery_location;
  }
  else
  {
    os << "(empty)";
  }
  os << " }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0601_airbase_locations,
  &klv_0601_airbase_locations::take_off_location,
  &klv_0601_airbase_locations::recovery_location
)

// ----------------------------------------------------------------------------
klv_0601_airbase_locations_format
::klv_0601_airbase_locations_format()
  : klv_data_format_< klv_0601_airbase_locations >{ 0 }
{}

// ----------------------------------------------------------------------------
std::string
klv_0601_airbase_locations_format
::description() const
{
  return "airbase locations pack of " + length_description();
}

// ----------------------------------------------------------------------------
klv_0601_airbase_locations
klv_0601_airbase_locations_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  klv_0601_airbase_locations result = {};
  auto const tracker = track_it( data, length );

  // Read take-off location
  auto const length_of_take_off_location =
    klv_read_ber< size_t >( data, tracker.remaining() );
  if( length_of_take_off_location )
  {
    // Take-off location is set
    result.take_off_location =
      klv_0601_location_dlp_format{}
        .read_( data, tracker.verify( length_of_take_off_location ) );
  }

  if( !tracker.remaining() )
  {
    // Recovery location is not included, set equal to take-off location
    result.recovery_location = result.take_off_location;
    return result;
  }

  // Read recovery location
  auto const length_of_recovery_location =
    klv_read_ber< size_t >( data, tracker.remaining() );
  // Recovery location is included
  if( length_of_recovery_location )
  {
    // Recovery location is set
    result.recovery_location =
      klv_0601_location_dlp_format{}
        .read_( data, tracker.verify( length_of_recovery_location ) );
  }

  return result;
}

// ----------------------------------------------------------------------------
void
klv_0601_airbase_locations_format
::write_typed( klv_0601_airbase_locations const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );

  // Write take-off location
  if( value.take_off_location )
  {
    // Take-off location is set
    size_t const length_of_take_off_location =
      klv_0601_location_dlp_format{}.length_of_( *value.take_off_location );

    if( length_of_take_off_location <= tracker.remaining() )
    {
      klv_write_ber( length_of_take_off_location, data, tracker.remaining() );

      klv_0601_location_dlp_format{}
        .write_( *value.take_off_location, data,
                 tracker.verify( length_of_take_off_location ) );
    }
  }
  else
  {
    // Take-off location is not set
    klv_write_ber< size_t >( 0, data, tracker.remaining() );
  }

  // Write recovery location
  if( !tracker.remaining() )
  {
    // Recovery location is not included
    return;
  }

  if( value.recovery_location )
  {
    // Recovery location is set
    if( value.recovery_location == value.take_off_location )
    {
      // Locations are the same, truncate the recovery location
      return;
    }

    size_t const length_of_recovery_location =
      klv_0601_location_dlp_format{}.length_of_( *value.recovery_location );

    if( length_of_recovery_location <= tracker.remaining() )
    {
      klv_write_ber( length_of_recovery_location, data, tracker.remaining() );

      klv_0601_location_dlp_format{}
        .write_( *value.recovery_location, data,
                 tracker.verify( length_of_recovery_location ) );
    }
  }
  else
  {
    // Recovery location is not set
    klv_write_ber< size_t >( 0, data, tracker.remaining() );
  }
}

// ----------------------------------------------------------------------------
size_t
klv_0601_airbase_locations_format
::length_of_typed( klv_0601_airbase_locations const& value ) const
{
  // Take-off location
  size_t length_of_take_off_location = 0;
  if( value.take_off_location )
  {
    // Latitude and longitude are required, altitude is not
    length_of_take_off_location =
      klv_0601_location_dlp_format{}.length_of_( *value.take_off_location );
  }

  auto length_of_length_of_take_off_location =
    klv_ber_length( length_of_take_off_location );

  // Recovery location
  size_t length_of_recovery_location = 0;
  if( value.recovery_location )
  {
    length_of_recovery_location =
      klv_0601_location_dlp_format{}.length_of_( *value.recovery_location );
  }

  auto length_of_length_of_recovery_location =
    klv_ber_length( length_of_recovery_location );

  if( value.recovery_location == value.take_off_location )
  {
    // Recovery is truncated
    length_of_length_of_recovery_location = 0;
    length_of_recovery_location = 0;
  }

  return length_of_length_of_take_off_location +
         length_of_take_off_location +
         length_of_length_of_recovery_location +
         length_of_take_off_location;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_view_domain_interval const& value )
{
  return os << "{ "
            << "start: " << value.start << ", "
            << "range: " << value.range << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0601_view_domain_interval,
  &klv_0601_view_domain_interval::start,
  &klv_0601_view_domain_interval::range
)

// ----------------------------------------------------------------------------
klv_imap_format const
klv_0601_view_domain_interval_format
::range_format{ vital::interval< double >{ 0.0, 360.0 } };

// ----------------------------------------------------------------------------
klv_0601_view_domain_interval_format
::klv_0601_view_domain_interval_format(
  vital::interval< double > const& start_interval )
  : klv_data_format_< klv_0601_view_domain_interval >{ 0 },
    m_start_format{ start_interval }
{}

// ----------------------------------------------------------------------------
std::string
klv_0601_view_domain_interval_format
::description() const
{
  return "view domain interval of " + length_description();
}

// ----------------------------------------------------------------------------
klv_0601_view_domain_interval
klv_0601_view_domain_interval_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  klv_0601_view_domain_interval result;
  if( length % 2 )
  {
    VITAL_THROW( kv::metadata_exception,
                 "odd length given for view domain interval" );
  }
  result.semi_length = length / 2;
  result.start = m_start_format.read_( data, result.semi_length ).value;
  result.range = range_format.read_( data, result.semi_length ).value;
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0601_view_domain_interval_format
::write_typed( klv_0601_view_domain_interval const& value,
               klv_write_iter_t& data, size_t length ) const
{
  if( length % 2 )
  {
    VITAL_THROW( kv::metadata_exception,
                 "odd length given for view domain interval" );
  }
  auto const semi_length = length / 2;
  m_start_format.write_( { value.start, semi_length }, data, semi_length );
  range_format.write_( { value.range, semi_length }, data, semi_length );
}

// ----------------------------------------------------------------------------
size_t
klv_0601_view_domain_interval_format
::length_of_typed( klv_0601_view_domain_interval const& value ) const
{
  return value.semi_length * 2;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_view_domain const& value )
{
  return os << "{ "
            << "azimuth: " << value.azimuth << ", "
            << "elevation: " << value.elevation << ", "
            << "roll: " << value.roll << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0601_view_domain,
  &klv_0601_view_domain::azimuth,
  &klv_0601_view_domain::elevation,
  &klv_0601_view_domain::roll
)

// ----------------------------------------------------------------------------
klv_0601_view_domain_interval_format const
klv_0601_view_domain_format
::azimuth_format{ vital::interval< double >{ 0.0, 360.0 } };

// ----------------------------------------------------------------------------
klv_0601_view_domain_interval_format const
klv_0601_view_domain_format
::elevation_format{ vital::interval< double >{ -180.0, 180.0 } };

// ----------------------------------------------------------------------------
klv_0601_view_domain_interval_format const
klv_0601_view_domain_format
::roll_format{ vital::interval< double >{ 0.0, 360.0 } };

// ----------------------------------------------------------------------------
klv_0601_view_domain_format
::klv_0601_view_domain_format()
  : klv_data_format_< klv_0601_view_domain >{ 0 }
{}

// ----------------------------------------------------------------------------
std::string
klv_0601_view_domain_format
::description() const
{
  return "view domain pack of " + length_description();
}

// ----------------------------------------------------------------------------
klv_0601_view_domain
klv_0601_view_domain_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_0601_view_domain result;
  result.azimuth =
    klv_read_trunc_lv( data, tracker.remaining(), azimuth_format );
  result.elevation =
    klv_read_trunc_lv( data, tracker.remaining(), elevation_format );
  result.roll =
    klv_read_trunc_lv( data, tracker.remaining(), roll_format );
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0601_view_domain_format
::write_typed( klv_0601_view_domain const& value,
               klv_write_iter_t& data, size_t length ) const
{
  klv_write_trunc_lv(
    std::tie( value.azimuth, value.elevation, value.roll ),
    data, length,
    azimuth_format, elevation_format, roll_format );
}

// ----------------------------------------------------------------------------
size_t
klv_0601_view_domain_format
::length_of_typed( klv_0601_view_domain const& value ) const
{
  return
    klv_length_of_trunc_lv(
      std::tie( value.azimuth, value.elevation, value.roll ),
      azimuth_format, elevation_format, roll_format );
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_waypoint_record const& value )
{
  os << "{ "
     << "ID: "
     << value.id
     << ", "
     << "prosecution order: "
     << value.order
     << ", "
     << "info: { "
     << "mode: ";
  auto const mode = *value.info & 1;
  os << ( mode
      ? "manual"
      : "automated" )
     << ", "
     << "source: ";
  auto const source = ( *value.info >> 1 ) & 1;
  os << ( source
      ? "ad hoc"
      : "pre-planned" )
     << " }"
     << ", "
     << "location: "
     << value.location
     << " } }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0601_waypoint_record,
  &klv_0601_waypoint_record::id,
  &klv_0601_waypoint_record::order,
  &klv_0601_waypoint_record::info,
  &klv_0601_waypoint_record::location
)

// ----------------------------------------------------------------------------
klv_0601_waypoint_record_format
::klv_0601_waypoint_record_format()
  : klv_data_format_< klv_0601_waypoint_record >{ 0 }
{}

// ----------------------------------------------------------------------------
std::string
klv_0601_waypoint_record_format
::description() const
{
  return "waypoint pack of " + length_description();
}

// ----------------------------------------------------------------------------
klv_0601_waypoint_record
klv_0601_waypoint_record_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  klv_0601_waypoint_record result = {};
  auto const tracker = track_it( data, length );

  // Read waypoint id
  result.id = klv_read_ber_oid< uint16_t >( data, tracker.remaining() );

  // Read waypoint order
  result.order = klv_read_int< int16_t >( data, tracker.verify( 2 ) );

  if( tracker.remaining() )
  {
    // Read waypoint info value
    result.info = klv_read_ber_oid< uint8_t >( data, tracker.verify( 1 ) );
  }

  if( tracker.remaining() )
  {
    // Read waypoint location
    result.location =
      klv_0601_location_dlp_format{}.read_( data, tracker.remaining() );
  }

  return result;
}

// ----------------------------------------------------------------------------
void
klv_0601_waypoint_record_format
::write_typed( klv_0601_waypoint_record const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );

  // Write waypoint id
  klv_write_ber_oid( value.id, data, tracker.remaining() );

  // Write waypoint order
  klv_write_int( value.order, data, tracker.verify( 2 ) );

  if( value.info )
  {
    // Write waypoint info value
    klv_write_ber_oid( *value.info, data, tracker.verify( 1 ) );
  }

  if( value.location && value.info )
  {
    // Write waypoint location
    klv_0601_location_dlp_format{}
      .write_( *value.location, data, tracker.remaining() );
  }
}

// ----------------------------------------------------------------------------
size_t
klv_0601_waypoint_record_format
::length_of_typed( klv_0601_waypoint_record const& value ) const
{
  size_t const length_of_waypoint_id = klv_ber_oid_length( value.id );

  size_t const length_of_waypoint_location =
    ( value.location && value.info )
    ? klv_0601_location_dlp_format{}.length_of_( *value.location )
    : 0;

  return ( length_of_waypoint_id + 2 + ( value.info ? 1 : 0 ) +
           length_of_waypoint_location );
}

// ----------------------------------------------------------------------------
/// Weapon/Store state ( General Status ).
std::ostream&
operator<<( std::ostream& os, klv_0601_weapon_general_status value )
{
  static std::string strings[ KLV_0601_WEAPON_GENERAL_STATUS_ENUM_END + 1 ] =
  {
    "Off",
    "Initialization",
    "Ready/Degraded",
    "Ready/All Up Round",
    "Launch",
    "Free Flight",
    "Abort",
    "Miss Fire",
    "Hang Fire",
    "Jettisoned",
    "Stepped Over",
    "No Status Available",
    "Unknown Weapons Store State" };

  os << strings[ std::min( value, KLV_0601_WEAPON_GENERAL_STATUS_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
/// A set of bit values to report the status of a weapon before it’s launched.
std::ostream&
operator<<( std::ostream& os, klv_0601_weapon_engagement_status_bit value )
{
  static std::string strings[ KLV_0601_WEAPON_ENGAGEMENT_STATUS_BIT_ENUM_END +
                              1 ] = {
    "Fuze Enabled",
    "Laser Enabled",
    "Target Enabled",
    "Weapon Armed",
    "Unknown Engagement Status Bit" };

  os << strings[ std::min( value,
                           KLV_0601_WEAPON_ENGAGEMENT_STATUS_BIT_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
/// List of weapon stores and status.
std::ostream&
operator<<( std::ostream& os, klv_0601_weapons_store const& value )
{
  os << "{ "
     << "station ID: "
     << value.station_id
     << ", "
     << "hardpoint ID: "
     << value.hardpoint_id
     << ", "
     << "carriage ID: "
     << value.carriage_id
     << ", "
     << "store ID: "
     << value.store_id
     << ", "
     << "general status: "
     << value.general_status
     << ", "
     << "engagement status: "
     << value.engagement_status
     << ", "
     << "weapon type: "
     << value.weapon_type
     << " }";

  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0601_weapons_store,
  &klv_0601_weapons_store::station_id,
  &klv_0601_weapons_store::hardpoint_id,
  &klv_0601_weapons_store::carriage_id,
  &klv_0601_weapons_store::store_id,
  &klv_0601_weapons_store::general_status,
  &klv_0601_weapons_store::engagement_status,
  &klv_0601_weapons_store::weapon_type
)

// ----------------------------------------------------------------------------
klv_0601_weapons_store_format
::klv_0601_weapons_store_format()
  : klv_data_format_< klv_0601_weapons_store >{ 0 }
{}

// ----------------------------------------------------------------------------
std::string
klv_0601_weapons_store_format
::description() const
{
  return "weapons store pack of " + length_description();
}

// ----------------------------------------------------------------------------
klv_0601_weapons_store
klv_0601_weapons_store_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  klv_0601_weapons_store result = {};
  auto const tracker = track_it( data, length );

  // Read weapon location
  result.station_id =
    klv_read_ber_oid< uint16_t >( data, tracker.remaining() );
  result.hardpoint_id =
    klv_read_ber_oid< uint16_t >( data, tracker.remaining() );
  result.carriage_id =
    klv_read_ber_oid< uint16_t >( data, tracker.remaining() );
  result.store_id =
    klv_read_ber_oid< uint16_t >( data, tracker.remaining() );

  // Read weapons status
  // Sets status = 0 0 engagement-status general-status
  auto status =
    klv_read_ber_oid< uint16_t >( data, tracker.remaining() );
  // General status = least significant 8 bits
  result.general_status =
    static_cast< klv_0601_weapon_general_status >( status & 0xFF );
  // Engagement status = next 4 bits
  result.engagement_status =
    bitfield_to_enums< klv_0601_weapon_engagement_status_bit, uint8_t >(
      ( status >> 8 ) & 0x0F );

  // Read weapons type
  auto const length_of_weapon_type =
    klv_read_ber< size_t >( data, tracker.remaining() );
  result.weapon_type =
    klv_read_string( data, length_of_weapon_type );

  return result;
}

// ----------------------------------------------------------------------------
void
klv_0601_weapons_store_format
::write_typed( klv_0601_weapons_store const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );

  // Write weapon location
  klv_write_ber_oid( value.station_id, data, tracker.remaining() );
  klv_write_ber_oid( value.hardpoint_id, data, tracker.remaining() );
  klv_write_ber_oid( value.carriage_id, data, tracker.remaining() );
  klv_write_ber_oid( value.store_id, data, tracker.remaining() );

  // Write weapons status
  // When the low order 7 bits of the MSB are zero, the MSB is eliminated
  auto const engagement_status_int =
    enums_to_bitfield< klv_0601_weapon_engagement_status_bit >(
      value.engagement_status );
  uint16_t status = ( engagement_status_int << 8 ) + value.general_status;
  klv_write_ber_oid( status, data, tracker.remaining() );

  // Write weapons type
  size_t const length_of_weapon_type =
  klv_string_length( value.weapon_type );
  klv_write_ber< size_t >( length_of_weapon_type, data,
                               tracker.remaining() );
  klv_write_string( value.weapon_type, data, tracker.remaining() );
}

// ----------------------------------------------------------------------------
size_t
klv_0601_weapons_store_format
::length_of_typed( klv_0601_weapons_store const& value ) const
{
  // Length of weapon location
  size_t const length_of_weapon_location =
    klv_ber_oid_length( value.station_id ) +
    klv_ber_oid_length( value.hardpoint_id ) +
    klv_ber_oid_length( value.carriage_id ) +
    klv_ber_oid_length( value.store_id );

  // Length of weapon status
  auto const engagement_status_int =
    enums_to_bitfield< klv_0601_weapon_engagement_status_bit >(
      value.engagement_status );
  uint16_t status = ( engagement_status_int << 8 ) + value.general_status;
  size_t const length_of_status = klv_ber_oid_length( status );

  // Length of weapon type
  size_t const length_of_weapon_type =
    klv_string_length( value.weapon_type );
  size_t const length_of_length_of_weapon_type =
    klv_ber_length( length_of_weapon_type );

  // Total length for record
  return ( length_of_weapon_location +
           length_of_status +
           length_of_weapon_type +
           length_of_length_of_weapon_type );
}

// ----------------------------------------------------------------------------
/// Optical sensors and non-optical payload package types.
std::ostream&
operator<<( std::ostream& os, klv_0601_payload_type const& value )
{
  static std::string strings[ KLV_0601_PAYLOAD_TYPE_ENUM_END + 1 ] =
  {
    "Electro Optical MI Sensor",
    "LIDAR",
    "RADAR",
    "SIGINT",
    "Unknown Payload Type"
  };

  os << strings[ std::min( value, KLV_0601_PAYLOAD_TYPE_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
/// Type, name, and id of a payload.
std::ostream&
operator<<( std::ostream& os, klv_0601_payload_record const& value )
{
  return os << "{ "
            << "ID: "
            << value.id
            << ", "
            << "type: "
            << value.type
            << ", "
            << "name: "
            << value.name
            << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0601_payload_record,
  &klv_0601_payload_record::id,
  &klv_0601_payload_record::type,
  &klv_0601_payload_record::name
)

// ----------------------------------------------------------------------------
/// Interprets data as a payload record.
klv_0601_payload_record_format
::klv_0601_payload_record_format()
  : klv_data_format_< klv_0601_payload_record >{ 0 }
{}

// ----------------------------------------------------------------------------
std::string
klv_0601_payload_record_format
::description() const
{
  return "payload pack of " + length_description();
}

// ----------------------------------------------------------------------------
klv_0601_payload_record
klv_0601_payload_record_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  klv_0601_payload_record result = {};
  auto const tracker = track_it( data, length );

  // Read payload id
  result.id = klv_read_ber_oid< uint16_t >( data, tracker.remaining() );

  // Read payload type
  result.type = static_cast< klv_0601_payload_type >(
    klv_read_ber_oid< uint16_t >( data, tracker.remaining() ) );

  // Read payload name
  auto const length_of_payload_name =
    klv_read_ber< size_t >( data, tracker.remaining() );
  result.name =
    klv_read_string( data, tracker.verify( length_of_payload_name ) );

  return result;
}

// ----------------------------------------------------------------------------
void
klv_0601_payload_record_format
::write_typed( klv_0601_payload_record const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );

  // Write payload id
  klv_write_ber_oid( value.id, data, tracker.remaining() );

  // Write payload type
  klv_write_ber_oid( static_cast< uint16_t >( value.type ),
                     data, tracker.remaining() );

  // Write payload name
  size_t const length_of_payload_name = klv_string_length( value.name );
  klv_write_ber< size_t >( length_of_payload_name, data,
                           tracker.remaining() );
  klv_write_string( value.name, data, tracker.remaining() );
}

// ----------------------------------------------------------------------------
size_t
klv_0601_payload_record_format
::length_of_typed( klv_0601_payload_record const& value ) const
{
  // Length of payload id
  size_t const length_of_payload_id = klv_ber_oid_length( value.id );

  // Length of payload type
  size_t const length_of_payload_type =
    klv_ber_oid_length( static_cast< uint16_t >( value.type ) );

  // Length of payload name
  size_t const length_of_payload_name = klv_string_length( value.name );
  size_t const length_of_length_of_payload_name =
    klv_ber_length( length_of_payload_name );

  return ( length_of_payload_id +
           length_of_payload_type +
           length_of_payload_name +
           length_of_length_of_payload_name );
}

// ----------------------------------------------------------------------------
/// Interprets data as a payload list.
klv_0601_payload_list_format
::klv_0601_payload_list_format()
  : klv_data_format_< std::vector< klv_0601_payload_record > >{ 0 }
{}

// ----------------------------------------------------------------------------
std::string
klv_0601_payload_list_format
::description() const
{
  return "payload list pack of " + length_description();
}

// ----------------------------------------------------------------------------
std::vector< klv_0601_payload_record >
klv_0601_payload_list_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  std::vector< klv_0601_payload_record > result = {};
  auto const tracker = track_it( data, length );

  // Read payload count
  auto count = klv_read_ber_oid< uint16_t >( data, tracker.remaining() );

  // Read payload list
  klv_series_format< klv_0601_payload_record_format > item;
  result = item.read_( data, length - klv_ber_oid_length( count ) );

  return result;
}

// ----------------------------------------------------------------------------
void
klv_0601_payload_list_format
::write_typed( std::vector< klv_0601_payload_record > const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );

  // Write payload count
  klv_write_ber_oid( value.size(), data, tracker.remaining() );

  // Write payload list
  klv_series_format< klv_0601_payload_record_format > item;
  item.write_( value, data, length - klv_ber_oid_length( value.size() ) );
}

// ----------------------------------------------------------------------------
size_t
klv_0601_payload_list_format
::length_of_typed( std::vector< klv_0601_payload_record > const& value ) const
{
  size_t total_length = 0;

  // Length of payload count
  total_length += klv_ber_oid_length( value.size() );

  // Length of payload list
  klv_series_format< klv_0601_payload_record_format > item;
  total_length += item.length_of_( value );
  return total_length;
}

// ----------------------------------------------------------------------------
/// A sensor wavelength record.
std::ostream&
operator<<( std::ostream& os, klv_0601_wavelength_record const& value )
{
  return os << "{ "
            << "ID: "
            << value.id
            << ", "
            << "minimum: "
            << value.min
            << ", "
            << "maximum: "
            << value.max
            << ", "
            << "name: "
            << value.name
            << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0601_wavelength_record,
  &klv_0601_wavelength_record::id,
  &klv_0601_wavelength_record::min,
  &klv_0601_wavelength_record::max,
  &klv_0601_wavelength_record::name
)

// ----------------------------------------------------------------------------
klv_0601_wavelength_record_format
::klv_0601_wavelength_record_format()
  : klv_data_format_< klv_0601_wavelength_record >{ 0 }
{}

// ----------------------------------------------------------------------------
std::string
klv_0601_wavelength_record_format
::description() const
{
  return "wavelength pack of " + length_description();
}

// ----------------------------------------------------------------------------
klv_0601_wavelength_record
klv_0601_wavelength_record_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  klv_0601_wavelength_record result = {};
  auto const tracker = track_it( data, length );

  // Read wavelength id
  result.id = klv_read_ber_oid< uint16_t >( data, tracker.remaining() );

  // Read min wavelength
  result.min = klv_read_imap( { 0.0, 1.0e9 }, data, 4 );

  // Read max wavelength
  result.max = klv_read_imap( { 0.0, 1.0e9 }, data, 4 );

  // Read wavelength name
  result.name = klv_read_string( data, tracker.remaining() );

  return result;
}

// ----------------------------------------------------------------------------
void
klv_0601_wavelength_record_format
::write_typed( klv_0601_wavelength_record const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );

  // Write wavelength id
  klv_write_ber_oid( value.id, data, tracker.remaining() );

  // Write min wavelength
  klv_write_imap( value.min, { 0.0, 1.0e9 }, data, 4 );

  // Write max wavelength
  klv_write_imap( value.max, { 0.0, 1.0e9 }, data, 4 );

  // Write wavelength name
  klv_write_string( value.name, data, tracker.remaining() );
}

// ----------------------------------------------------------------------------
size_t
klv_0601_wavelength_record_format
::length_of_typed( klv_0601_wavelength_record const& item ) const
{
  // Length of wavelength id
  size_t const length_of_wavelength_id = klv_ber_oid_length( item.id );

  // Length of wavelength name
  size_t const length_of_wavelength_name = klv_string_length( item.name );

  return ( length_of_wavelength_id + 8 + length_of_wavelength_name );
}

// ----------------------------------------------------------------------------
klv_checksum_packet_format const*
klv_0601_local_set_format
::checksum_format() const
{
  return &m_checksum_format;
}

// ----------------------------------------------------------------------------
std::string
klv_0601_local_set_format
::description() const
{
  return "UAS datalink local set of " + length_description();
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0601_tag tag )
{
  return os << klv_0601_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
klv_0601_local_set_format
::klv_0601_local_set_format()
  : klv_local_set_format{ klv_0601_traits_lookup() },
    m_checksum_format{ { KLV_0601_CHECKSUM, 2 } }
{}

// ----------------------------------------------------------------------------
klv_uds_key
klv_0601_key()
{
  // From Section 6.2 of
  // https://gwg.nga.mil/misb/docs/standards/ST0601.17.pdf
  return { 0x060E2B34020B0101, 0x0E01030101000000 };
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
