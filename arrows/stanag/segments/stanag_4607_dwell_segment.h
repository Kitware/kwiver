// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Defines a STANAG 4607 segment header and the various segment types

#ifndef KWIVER_ARROWS_STANAG_4607_DWELL_SEGMENT_H_
#define KWIVER_ARROWS_STANAG_4607_DWELL_SEGMENT_H_

#include "stanag_4607_segments.h"
#include <arrows/stanag/kwiver_algo_stanag_export.h>
#include <arrows/stanag/stanag_util.h>

#include <arrows/stanag/stanag_util.h>
#include <vital/optional.h>

#include <initializer_list>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>
#include <optional>

namespace kwiver {

namespace arrows {

namespace stanag {

namespace kv = kwiver::vital;


// ----------------------------------------------------------------------------
/// Position of the sensor at the temporal center of the dwell
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_sensor_position
{
  double latitude;
  double longitude;
  int altitude;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_sensor_position const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_sensor_position )

// ----------------------------------------------------------------------------
/// A factor which modifies the value of the reported targetposition (lat, lon)
/// when it is necessary to send the reduced bandwidth version of the Target
/// Report.
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_scale_factor
{
  int lat_scale;
  int long_scale;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_scale_factor const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_scale_factor )

// ----------------------------------------------------------------------------
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_scale_factor_format
{
public:
  stanag_4607_scale_factor_format() {}

  stanag_4607_scale_factor
  read( ptr_t& ptr ) const;
};


// ----------------------------------------------------------------------------
/// Estimate of the standard deviation in the estimated sensor location at
///  the time of the dwell. Expressed in centimeters.
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_sensor_pos_uncert
{
  int along_track;
  int cross_track;
  int altitude;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_sensor_pos_uncert const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_sensor_pos_uncert )

// ----------------------------------------------------------------------------
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_sensor_pos_uncert_format
{
public:
  stanag_4607_sensor_pos_uncert_format() {}

  stanag_4607_sensor_pos_uncert
  read( ptr_t& ptr ) const;
};


// ----------------------------------------------------------------------------
/// The toll angle of the platform at the time of the dwell.
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_orientation
{
  double heading;
  double pitch;
  double roll;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_orientation const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_orientation )

// ----------------------------------------------------------------------------
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_orientation_format
{
public:
  stanag_4607_orientation_format() {}

  stanag_4607_orientation
  read( ptr_t& ptr ) const;
};


// ----------------------------------------------------------------------------
/// The position of the center of the dwell area
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_dwell_area
{
  double center_lat;
  double center_long;
  double range_half_ext;
  double dwell_angle_half_ext;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_dwell_area const& value );


// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_dwell_area )

// ----------------------------------------------------------------------------
/// Classification of the target.
enum KWIVER_ALGO_STANAG_EXPORT stanag_4607_target_classification
{
  STANAG_4607_TARGET_CLASS_NO_INFO_LIVE,
  STANAG_4607_TARGET_CLASS_TRACKED_VEHICLE_LIVE,
  STANAG_4607_TARGET_CLASS_WHEELED_VEHICLE_LIVE,
  STANAG_4607_TARGET_CLASS_ROTARY_WING_AIRCRAFT_LIVE,
  STANAG_4607_TARGET_CLASS_FIXED_WING_AIRCRAFT_LIVE,
  STANAG_4607_TARGET_CLASS_STATIONARY_ROTATOR_LIVE,
  STANAG_4607_TARGET_CLASS_MARITIME_LIVE,
  STANAG_4607_TARGET_CLASS_BEACON_LIVE,
  STANAG_4607_TARGET_CLASS_AMPHIBIOUS_LIVE,
  STANAG_4607_TARGET_CLASS_PERSON_LIVE,
  STANAG_4607_TARGET_CLASS_VEHICLE_LIVE,
  STANAG_4607_TARGET_CLASS_ANIMAL_LIVE,
  STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_LIVE_LAND,
  STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_LIVE_MARITIME,
  // Note: 14-125 are reserved
  STANAG_4607_TARGET_CLASS_OTHER_LIVE                       = 126,
  STANAG_4607_TARGET_CLASS_UNKNOWN_LIVE,
  STANAG_4607_TARGET_CLASS_NO_INFO_SIM,
  STANAG_4607_TARGET_CLASS_TRACKED_VEHICLE_SIM,
  STANAG_4607_TARGET_CLASS_WHEELED_VEHICLE_SIM,
  STANAG_4607_TARGET_CLASS_ROTARY_WING_AIRCRAFT_SIM,
  STANAG_4607_TARGET_CLASS_FIXED_WING_AIRCRAFT_SIM,
  STANAG_4607_TARGET_CLASS_STATIONARY_ROTATOR_SIM,
  STANAG_4607_TARGET_CLASS_MARITIME_SIM,
  STANAG_4607_TARGET_CLASS_BEACON_SIM,
  STANAG_4607_TARGET_CLASS_AMPHIBIOUS_SIM,
  STANAG_4607_TARGET_CLASS_PERSON_SIM,
  STANAG_4607_TARGET_CLASS_VEHICLE_SIM,
  STANAG_4607_TARGET_CLASS_ANIMAL_SIM,
  STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_SIM_LAND,
  STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_SIM_MARITIME,
  STANAG_4607_TARGET_CLASS_TAGGING_DEVICE,
  // Note: 143-253 are reserved
  STANAG_4607_TARGET_CLASS_OTHER_SIM                         = 254,
  STANAG_4607_TARGET_CLASS_UNKNOWN_SIM,
  STANAG_4607_TARGET_CLASS_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os,
            stanag_4607_target_classification const& value );

// ----------------------------------------------------------------------------
/// Standard deviation of the target measurements.
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_target_measure_uncert
{
  int slant_range;
  int cross_range;
  int height;
  int radial_velocity;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_target_measure_uncert const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_target_measure_uncert )

// ----------------------------------------------------------------------------
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_target_measure_uncert_format
{
public:
  stanag_4607_target_measure_uncert_format() {}

  stanag_4607_target_measure_uncert
  read( ptr_t& ptr ) const;
};


// ----------------------------------------------------------------------------
/// Information used to generate the MTI Target.
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_truth_tag
{
  int application;
  int entity;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_truth_tag const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_truth_tag )

// ----------------------------------------------------------------------------
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_truth_tag_format
{
public:
  stanag_4607_truth_tag_format() {}

  stanag_4607_truth_tag
  read( ptr_t& ptr ) const;
};


// ----------------------------------------------------------------------------
/// Each bit of the Existence Mask indicates whether or not the corresponding
/// field of the Dwell Segment is present in the data stream.
enum KWIVER_ALGO_STANAG_EXPORT stanag_4607_dwell_existence_mask_bit
{
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_RADAR_CROSS_SECT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_TRUTH_TAG_ENTITY,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_TRUTH_TAG_APPL,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_MEASURE_RADIAL_VEL,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_MEASURE_HEIGHT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_MEASURE_CROSS_RNAGE,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_MEASURE_SLANT_RANGE,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_CLASS_PROB,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_CLASS,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_SNR,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_WRAP_VEL,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_VEL_LOS,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_GEODETIC_HEIGHT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_DELTA_LONG,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_DELTA_LAT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_HI_RES_LONG,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_HI_RES_LAT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_MTI_REPORT_IDX,
  STANAG_4607_DWELL_EXIST_MASK_BIT_MIN_DETECT_VEL,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_ORIENT_ROLL,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_OREINT_PITCH,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_OREINT_HEADING,
  STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_AREA_DWELL_ANGLE_HALF,
  STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_AREA_RANGE_HALF,
  STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_AREA_CENTER_LONG,
  STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_AREA_CENTER_LAT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_PLATFORM_OREINT_ROLL,
  STANAG_4607_DWELL_EXIST_MASK_BIT_PLATFORM_ORIENT_PITCH,
  STANAG_4607_DWELL_EXIST_MASK_BIT_PLATFORM_OREINT_HEADING,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_VERTICAL_VEL_UNCERT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_SPEED_UNCERT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_TRACK_UNCERT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_VERTICAL_VEL,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_SPEED,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_TRACK,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_POS_ALT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_POS_CROSS_TRACK,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_POS_ALONG_TRACK,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SCALE_FACT_LONG,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SCALE_FACT_LAT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_ALT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_LONG,
  STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_LAT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_TIME,
  STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_COUNT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_LAST_DWELL_REVISIT,
  STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_INDEX,
  STANAG_4607_DWELL_EXIST_MASK_BIT_REVISIT_INDEX,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os,
            stanag_4607_dwell_existence_mask_bit const& value );

// ----------------------------------------------------------------------------
/// The position of the reported detection.
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_target_location
{
  std::optional< double > hi_res_lat;
  std::optional< double > hi_res_long;
  std::optional< int > delta_lat;
  std::optional< int > delta_long;
  std::optional< int > geodetic_height;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_target_location const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_target_location )

// ----------------------------------------------------------------------------
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_target_location_format
{
public:
  stanag_4607_target_location_format() {}

  stanag_4607_target_location
  read( ptr_t& ptr,
        std::set< stanag_4607_dwell_existence_mask_bit > existence_mask ) const;
};


// ----------------------------------------------------------------------------
/// Each target observed within the dwell.
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_target_report
{
  std::optional< int > mti_report_idx;
  std::optional< stanag_4607_target_location > location;
  std::optional< int > velocity_los;
  std::optional< int > wrap_velocity;
  std::optional< int > snr;
  std::optional< stanag_4607_target_classification > classification;
  std::optional< int > class_probability;
  std::optional< stanag_4607_target_measure_uncert > measurement_uncert;
  std::optional< stanag_4607_truth_tag > truth_tag;
  std::optional< int > radar_cross_sect;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_target_report const& value );

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os,
            std::vector< stanag_4607_target_report > const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_target_report )

// ----------------------------------------------------------------------------
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_target_report_format
{
public:
  stanag_4607_target_report_format();

  stanag_4607_target_report
  read( ptr_t& ptr,
      std::set< stanag_4607_dwell_existence_mask_bit > existence_mask ) const;
};


// ----------------------------------------------------------------------------
/// A report on a grouping of zero or more target reports.
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_dwell_segment
{
  std::set< stanag_4607_dwell_existence_mask_bit > existence_mask;
  int revisit_index;
  int dwell_index;
  int last_dwell_of_revisit;
  int target_report_count;
  int dwell_time;
  stanag_4607_sensor_position sensor_position;
  std::optional< stanag_4607_scale_factor > scale_factor;
  std::optional< stanag_4607_sensor_pos_uncert > sensor_pos_uncert;
  std::optional< double > sensor_track;
  std::optional< int > sensor_speed;
  std::optional< int > sensor_vertical_vel;
  std::optional< int > sensor_track_uncert;
  std::optional< int > sensor_speed_uncert;
  std::optional< int > sensor_vertical_vel_uncert;
  std::optional< stanag_4607_orientation > platform_orient;
  stanag_4607_dwell_area dwell_area;
  std::optional< stanag_4607_orientation > sensor_orientation;
  std::optional< int > min_detectable_vel;
  std::vector< stanag_4607_target_report > target_reports;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_dwell_segment const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_dwell_segment )

// ----------------------------------------------------------------------------
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_dwell_segment_format
  : public stanag_4607_segment_data_format_< stanag_4607_dwell_segment >
{
public:
  stanag_4607_dwell_segment_format();


  size_t size;

  stanag_4607_dwell_segment
  read( ptr_t& ptr ) const;
};

} // namespace stanag

} // namespace arrows

} // namespace kwiver

#endif
