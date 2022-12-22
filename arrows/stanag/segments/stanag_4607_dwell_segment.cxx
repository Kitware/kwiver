// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "stanag_4607_dwell_segment.h"

namespace ka = kwiver::arrows;

#include <vital/exceptions/base.h>
#include <vital/util/interval.h>


#include <ostream>
#include <initializer_list>
#include <vector>
#include <iostream>
#include <limits>

namespace kwiver {

namespace arrows {

namespace stanag {


// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_sensor_position const& value )
{
  return os << "{ "
            << "Latitude: " << value.latitude << " degrees" << ", "
            << "Longitude: " << value.longitude << " degrees" << ", "
            << "Altitude: " << value.altitude << " cm"
            << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP (
  stanag_4607_sensor_position,
  &stanag_4607_sensor_position::latitude,
  &stanag_4607_sensor_position::longitude,
  &stanag_4607_sensor_position::altitude
)

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_scale_factor const& value )
{
  return os << "{ "
            << "Lat Scale: " << value.lat_scale << " degrees" << ", "
            << "Long Scale: " << value.long_scale << " degrees"
            << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP (
  stanag_4607_scale_factor,
  &stanag_4607_scale_factor::lat_scale,
  &stanag_4607_scale_factor::long_scale
)

// ----------------------------------------------------------------------------
stanag_4607_scale_factor
stanag_4607_scale_factor_format
::read( ptr_t& ptr ) const
{
  stanag_4607_scale_factor result;

  result.lat_scale = klv::klv_read_int< int >( ptr, (size_t)4 );
  result.long_scale = klv::klv_read_int< int >( ptr, (size_t)4 );

  return result;

}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_sensor_pos_uncert const& value )
{
  return os << "{ "
            << "Along Track: " << value.along_track << " cm" << ", "
            << "Cross Track: " << value.cross_track << " cm" << ", "
            << "Altitude: " << value.altitude << " cm"
            << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP (
  stanag_4607_sensor_pos_uncert,
  &stanag_4607_sensor_pos_uncert::along_track,
  &stanag_4607_sensor_pos_uncert::cross_track,
  &stanag_4607_sensor_pos_uncert::altitude
)

// ----------------------------------------------------------------------------
stanag_4607_sensor_pos_uncert
stanag_4607_sensor_pos_uncert_format
::read( ptr_t& ptr ) const
{
  stanag_4607_sensor_pos_uncert result;

  result.along_track = klv::klv_read_int< int >( ptr, (size_t)4 );
  result.cross_track = klv::klv_read_int< int >( ptr, (size_t)4 );
  result.altitude = klv::klv_read_int< int >( ptr, (size_t)2 );

  return result;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_orientation const& value )
{
  return os << "{ "
            << "Heading: " << value.heading << " degrees" << ", "
            << "Pitch: " << value.pitch << " degrees" << ", "
            << "Roll: " << value.roll << " degrees"
            << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP (
  stanag_4607_orientation,
  &stanag_4607_orientation::heading,
  &stanag_4607_orientation::pitch,
  &stanag_4607_orientation::roll
)

// ----------------------------------------------------------------------------
stanag_4607_orientation
stanag_4607_orientation_format
::read( ptr_t& ptr ) const
{
  stanag_4607_orientation result;

  result.heading = klv::klv_read_flint< uint16_t >({0, 369.9945}, ptr, 2);
  result.pitch = klv::klv_read_flint< int16_t >({-90, 90}, ptr, 2);
  result.roll = klv::klv_read_flint< int16_t >({-90, 90}, ptr, 2);

  return result;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_dwell_area const& value )
{
  return os << "{ "
            << "Center Latitude: " << value.center_lat << " degrees" << ", "
            << "Center Longitude: " << value.center_long << " degrees" << ", "
            << "Range Half Extent: " << value.range_half_ext << " km" << ", "
            << "Dwell Angle Half Extent: " << value.dwell_angle_half_ext
            << " degrees"
            << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP (
  stanag_4607_dwell_area,
  &stanag_4607_dwell_area::center_lat,
  &stanag_4607_dwell_area::center_long,
  &stanag_4607_dwell_area::range_half_ext,
  &stanag_4607_dwell_area::dwell_angle_half_ext
)

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os,
            stanag_4607_target_classification const& value )
{
  std::map< stanag_4607_target_classification, std::string > strings
  {
    { STANAG_4607_TARGET_CLASS_NO_INFO_LIVE, "No Information, Live Target" },
    { STANAG_4607_TARGET_CLASS_TRACKED_VEHICLE_LIVE,
      "Tracked Vehicle, Live Target" },
    { STANAG_4607_TARGET_CLASS_WHEELED_VEHICLE_LIVE,
      "Wheeled Vehicle, Live Target" },
    { STANAG_4607_TARGET_CLASS_ROTARY_WING_AIRCRAFT_LIVE,
      "Rotary Wing Aircraft, Live Target"} ,
    { STANAG_4607_TARGET_CLASS_FIXED_WING_AIRCRAFT_LIVE,
      "Fixed Wing Aircraft, Live Target" },
    { STANAG_4607_TARGET_CLASS_STATIONARY_ROTATOR_LIVE,
      "Stationary Rotator, Live Target" },
    { STANAG_4607_TARGET_CLASS_MARITIME_LIVE, "Maritime, Live Target" },
    { STANAG_4607_TARGET_CLASS_BEACON_LIVE, "Beacon, Live Target" },
    { STANAG_4607_TARGET_CLASS_AMPHIBIOUS_LIVE, "Amphibious, Live Target" },
    { STANAG_4607_TARGET_CLASS_PERSON_LIVE, "Person, Live Target" },
    { STANAG_4607_TARGET_CLASS_VEHICLE_LIVE, "Vehicle, Live Target" },
    { STANAG_4607_TARGET_CLASS_ANIMAL_LIVE, "Animal, Live Target" },
    { STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_LIVE_LAND,
      "Large Multiple-Return, Live Land Target" },
    { STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_LIVE_MARITIME,
      "Large Multiple-Return, Live Maritime Target" },
    { STANAG_4607_TARGET_CLASS_OTHER_LIVE, "Other, Live Target" },
    { STANAG_4607_TARGET_CLASS_UNKNOWN_LIVE, "Unknown, Live Target" },
    { STANAG_4607_TARGET_CLASS_NO_INFO_SIM,
      "No Information, Simulated Target" },
    { STANAG_4607_TARGET_CLASS_TRACKED_VEHICLE_SIM,
      "Tracked Vehicle, Simulated Target" },
    { STANAG_4607_TARGET_CLASS_WHEELED_VEHICLE_SIM,
      "Wheeled Vehicle, Simulated Target" },
    { STANAG_4607_TARGET_CLASS_ROTARY_WING_AIRCRAFT_SIM,
      "Rotary Wing Aircraft, Simulated Target" },
    { STANAG_4607_TARGET_CLASS_FIXED_WING_AIRCRAFT_SIM,
      "Fixed Wing Aircraft, Simulated Target" },
    { STANAG_4607_TARGET_CLASS_STATIONARY_ROTATOR_SIM,
      "Stationary Rotator, Simulated Target" },
    { STANAG_4607_TARGET_CLASS_MARITIME_SIM, "Maritime, Simulated Target" },
    { STANAG_4607_TARGET_CLASS_BEACON_SIM, "Beacon, Simulated Target" },
    { STANAG_4607_TARGET_CLASS_AMPHIBIOUS_SIM,
      "Amphibious, Simulated Target" },
    { STANAG_4607_TARGET_CLASS_PERSON_SIM, "Person, Simulated Target" },
    { STANAG_4607_TARGET_CLASS_VEHICLE_SIM, "Vehicle, Simulated Target" },
    { STANAG_4607_TARGET_CLASS_ANIMAL_SIM, "Animal, Simulated Target" },
    { STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_SIM_LAND,
      "Large Multiple-Return, Simulated Land Target" },
    { STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_SIM_MARITIME,
      "Large Multiple-Return, Simulated Maritime Target" },
    { STANAG_4607_TARGET_CLASS_TAGGING_DEVICE, "Tagging Device" },
    { STANAG_4607_TARGET_CLASS_OTHER_SIM, "Other, Simulated Target" },
    { STANAG_4607_TARGET_CLASS_UNKNOWN_SIM, "Unknown, Simulated Target" },
    { STANAG_4607_TARGET_CLASS_ENUM_END, "Unknown Target Classification" }
  };

  os << strings[ std::min( value, STANAG_4607_TARGET_CLASS_ENUM_END ) ];

  return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_target_measure_uncert const& value )
{
  return os << "{ "
            << "Slant Range: " << value.slant_range << " cm" << ", "
            << "Cross Range: " << value.cross_range << " dm" << ", "
            << "Height: " << value.height << " m" << ", "
            << "Target Radial Velocity: " << value.radial_velocity << " cm/sec"
            << " }";

}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP (
  stanag_4607_target_measure_uncert,
  &stanag_4607_target_measure_uncert::slant_range,
  &stanag_4607_target_measure_uncert::cross_range,
  &stanag_4607_target_measure_uncert::height,
  &stanag_4607_target_measure_uncert::radial_velocity
)
// ----------------------------------------------------------------------------
stanag_4607_target_measure_uncert
stanag_4607_target_measure_uncert_format
::read( ptr_t& ptr ) const
{
  stanag_4607_target_measure_uncert result;

  result.slant_range = klv::klv_read_int< int >( ptr, (size_t)2 );
  result.cross_range = klv::klv_read_int< int >( ptr, (size_t)2 );
  result.height = klv::klv_read_int< int >( ptr, (size_t)1 );
  result.radial_velocity = klv::klv_read_int< int >( ptr, (size_t)2 );

  return result;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_truth_tag const& value )
{
  return os << "{ "
            << "Application: " << value.application << ", "
            << "Entity: " << value.entity
            << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP (
  stanag_4607_truth_tag,
  &stanag_4607_truth_tag::application,
  &stanag_4607_truth_tag::entity
)

// ----------------------------------------------------------------------------
stanag_4607_truth_tag
stanag_4607_truth_tag_format
::read( ptr_t& ptr ) const
{
  stanag_4607_truth_tag result;

  result.application = klv::klv_read_int< int >( ptr, (size_t)1 );
  result.entity = klv::klv_read_int< int >( ptr, (size_t)4 );

  return result;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os,
            stanag_4607_dwell_existence_mask_bit const& value )
{
  static std::string strings[ STANAG_4607_DWELL_EXIST_MASK_BIT_REVISIT_INDEX
                              + 1 ] =
  {
    "Target Report: Radar Cross Section Transmitted",
    "Target Report: Truth Tag: Entity Transmitted",
    "Target Report: Truth Tag: Application Transmitted",
    "Target Report: Measurement Uncertainty: Radial Velocity Transmitted",
    "Target Report: Measurement Uncertainty: Height Transmitted",
    "Target Report: Measurement Uncertainty: Cross Range Transmitted",
    "Target Report: Measurement Uncertainty: Slant Range Transmitted",
    "Target Report: Class. Probability Transmitted",
    "Target Report: Classification Transmitted",
    "Target Report: SNR Transmitted",
    "Target Report: Wrap Velocity Transmitted",
    "Target Report: Velocity Line-of-Sight Component Transmitted",
    "Target Report: Location: Geodetic Height Transmitted",
    "Target Report: Location: Delta Long Transmitted",
    "Target Report: Location: Delta Lat Transmitted",
    "Target Report: Location: Hi-Res Longitude Transmitted",
    "Target Report: Location: Hi-Res Latitude Transmitted",
    "Target Report: MTI Report Index Transmitted",
    "Minimum Detectable Velocity Transmitted",
    "Sensor Orientation: Roll Transmitted",
    "Sensor Orientation: Pitch Transmitted",
    "Sensor Orientation: Heading Transmitted",
    "Dwell Area: Dwell Angle Half Extent Transmitted",
    "Dwell Area: Range Half Extent Transmitted",
    "Dwell Area: Center Longitude Transmitted",
    "Dwell Area: Center Latitude Transmitted",
    "Platform Orientation: Roll Transmitted",
    "Platform Orientation: Pitch Transmitted",
    "Platform Orientation: Heading Transmitted",
    "Sensor Vertical Velocity Uncertainty Transmitted",
    "Sensor Speed Uncertainty Transmitted",
    "Sensor Track Uncertainty Transmitted",
    "Sensor Vertical Velocity Transmitted",
    "Sensor Speed Transmitted",
    "Sensor Track Transmitted",
    "Sensor Position Uncertainty: Altitude Transmitted",
    "Sensor Position Uncertainty: Cross Track Transmitted",
    "Sensor Position Uncertainty: Along Track Transmitted",
    "Scale Factor: Long Scale Transmitted",
    "Scale Factor: Lat Scale Transmitted",
    "Sensor Position: Altitude Transmitted",
    "Sensor Position: Longitude Transmitted",
    "Sensor Position: Latitude Transmitted",
    "Dwell Time Transmitted",
    "Target Report Count Transmitted",
    "Last Dwell of Revisit Transmitted",
    "Dwell Index Transmitted",
    "Revisit Index Transmitted"
  };

  os << strings[ value ];
  return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_target_location const& value )
{
  os << "{ ";

  if( value.hi_res_lat )
  {
    os << "Hi-Res Latitude: " << *value.hi_res_lat << " degrees" << ", "
       << "Hi-Res Longitude: " << *value.hi_res_long << " degrees" << ", ";
  }

  if( value.delta_lat )
  {
    os << "Delta Lat: " << *value.delta_lat << ", "
       << "Delta Long: " << *value.delta_long << ", ";
  }

  if( value.geodetic_height )
  {
    os << "Geodetic Height: " << *value.geodetic_height << " m";
  }

  os << " }";

  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP (
  stanag_4607_target_location,
  &stanag_4607_target_location::hi_res_lat,
  &stanag_4607_target_location::hi_res_long,
  &stanag_4607_target_location::delta_lat,
  &stanag_4607_target_location::delta_long,
  &stanag_4607_target_location::geodetic_height
)

// ----------------------------------------------------------------------------
stanag_4607_target_location
stanag_4607_target_location_format
::read( ptr_t& ptr,
        std::set< stanag_4607_dwell_existence_mask_bit > existence_mask ) const
{
  stanag_4607_target_location result;

  // Fields D32.2-D32.3 are conditional and always sent together
  if( existence_mask.count(
          STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_HI_RES_LAT) )
  {
    result.hi_res_lat = klv::klv_read_flint< int32_t >({-90, 90}, ptr, 4);
    result.hi_res_long = klv::klv_read_flint< uint32_t >({0, 359.999999916},
                                                         ptr, 4);
  }
  // Fields D32.4-D32.5 are conditional and always sent together
  // Condition: Sent if D32.2 and D32.3 are not sent
  else{
    result.delta_lat = klv::klv_read_int< int >( ptr, (size_t)2 );
    result.delta_long = klv::klv_read_int< int >( ptr, (size_t)2 );
  }

  // Field D32.6 is optional
  if( existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_GEODETIC_HEIGHT)
    )
  {
    result.geodetic_height = klv::klv_read_int< int >( ptr, (size_t)2 );
  }

  return result;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_target_report const& value )
{
  os << "{ ";
  if ( value.mti_report_idx )
  {
    os << "MTI Report Index: " << *value.mti_report_idx << ", ";
  }

  if( value.location )
  {
    os << "Target Location: " << *value.location << ", ";
  }

  if( value.velocity_los )
  {
    os << "Target Velocity Line-of-Sight Component: " << *value.velocity_los
       << " cm/sec" << ", "
       << "Target Wrap Velocity: " << *value.wrap_velocity << " cm/sec"
       << ", ";
  }

  if( value.snr )
  {
    os << "Target SNR: " << *value.snr << " dB" << ", ";
  }

  if( value.classification )
  {
    os << "Target Classification: " << *value.classification << ", ";
  }

  if( value.class_probability )
  {
    os << "Target Class. Probability: " << *value.class_probability << " %"
       << ", ";
  }

  if( value.measurement_uncert )
  {
    os << "Target Measurement Uncertainty: " << *value.measurement_uncert
       << ", ";
  }

  if( value.truth_tag )
  {
    os << "Truth Tag: " << *value.truth_tag << ", ";
  }

  if( value.radar_cross_sect )
  {
    os << "Target Radar Cross Section:" <<  *value.radar_cross_sect
       << " dB/2";
  }

  os << " }";
  return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os,
            std::vector< stanag_4607_target_report > const& value )
{
  auto num_targets = value.size();

  os << "{ ";

  if( num_targets == 0 )
  {
    return os << "(empty) }";
  }

  for( size_t i=0; i<num_targets; i++ )
  {
    os << value[i] << ", ";
  }
  os << " }";

  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP(
  stanag_4607_target_report,
  &stanag_4607_target_report::mti_report_idx,
  &stanag_4607_target_report::location,
  &stanag_4607_target_report::velocity_los,
  &stanag_4607_target_report::wrap_velocity,
  &stanag_4607_target_report::snr,
  &stanag_4607_target_report::classification,
  &stanag_4607_target_report::class_probability,
  &stanag_4607_target_report::measurement_uncert,
  &stanag_4607_target_report::truth_tag,
  &stanag_4607_target_report::radar_cross_sect
)

// ----------------------------------------------------------------------------
stanag_4607_target_report_format
::stanag_4607_target_report_format()
{}

// ----------------------------------------------------------------------------
stanag_4607_target_report
stanag_4607_target_report_format
::read( ptr_t& ptr,
        std::set< stanag_4607_dwell_existence_mask_bit > existence_mask ) const
{
  stanag_4607_target_report result;

  // Field D32.1 is conditional
  // Condition: Send if an HRR report is provided for targets in this dwell
  if( existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_MTI_REPORT_IDX) )
  {
    result.mti_report_idx = klv::klv_read_int< int >( ptr, (size_t)2 );
  }

  result.location = stanag_4607_target_location_format{}
                    .read( ptr, existence_mask );

  // Fields D32.7-D32.8 are optional and always sent together
  if( existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_VEL_LOS) )
  {
    result.velocity_los = klv::klv_read_int< int >( ptr, (size_t)2 );
    result.wrap_velocity = klv::klv_read_int< int >( ptr, (size_t)2 );
  }

  // Field D32.9 is optional
  if( existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_SNR) )
  {
    result.snr = klv::klv_read_int< int >( ptr, (size_t)1 );
  }

  // Field D32.10 is optional
  if( existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_CLASS) )
  {
    result.classification = static_cast< stanag_4607_target_classification >(
        klv::klv_read_int< uint64_t >( ptr, (size_t)1 ));
  }

  // Field D32.11 is optional
  if( existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_CLASS_PROB) )
  {
    result.class_probability = klv::klv_read_int< int >( ptr, (size_t)1 );
  }

  // Fields D32.12-D32.15 are conditional and always sent together
  if( existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_MEASURE_SLANT_RANGE) )
  {
    result.measurement_uncert = stanag_4607_target_measure_uncert_format{}
                                .read( ptr );
  }

  // Fields D32.16-D32.17 are conditional and always sent together
  if( existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_TRUTH_TAG_APPL) )
  {
    result.truth_tag = stanag_4607_truth_tag_format{}.read( ptr );
  }

  // Field D32.18 is optional
  if( existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_RADAR_CROSS_SECT) )
  {
    result.radar_cross_sect = klv::klv_read_int< int >( ptr, (size_t)1 );
  }

  return result;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_dwell_segment const& value )
{
  os << "{ "
     << "Existence Mask: { ";
  for( stanag_4607_dwell_existence_mask_bit m : value.existence_mask )
  {
    os << m << ", ";
  }
  os << " }, ";

  os << "Revisit Index: " << value.revisit_index << ", "
     << "Dwell Index: " << value.dwell_index << ", "
     << "Last Dwell of Revisit: "
     << ( value.last_dwell_of_revisit
        ? "No additional dwells"
        : "Additional dwells" )
     << ", "
     << "Target Report Count: " << value.target_report_count << ", "
     << "Dwell Time: " << value.dwell_time << " ms" << ", "
     << "Sensor Position: " << value.sensor_position << ", ";
  if( value.scale_factor )
  {
    os << "Scale Factor: " << *value.scale_factor << ", ";
  }

  if( value.sensor_pos_uncert )
  {
    os << "Sensor Position Uncertainty: " << *value.sensor_pos_uncert << ", ";
  }

  if( value.sensor_track )
  {
    os << "Sensor Track: " << *value.sensor_track << " degrees" << ", "
       << "Sensor Speed: " << *value.sensor_speed << " mm/sec" << ", "
       << "Sensor Vertical Velocity: " << *value.sensor_vertical_vel
       << " dm/sec" << ", ";
  }

  if( value.sensor_track_uncert )
  {
    os << "Sensor Track Uncertainty: " << *value.sensor_track_uncert
       << " degrees" << ", "
       << "Sensor Speed Uncertainty: " << *value.sensor_speed_uncert
       << " mm/sec" << ", "
       << "Sensor Vertical Velocity Uncertainty: "
       << *value.sensor_vertical_vel_uncert << " cm/sec" << ", ";
  }

  if( value.platform_orient )
  {
    os << "Platform Orientation: " << *value.platform_orient << ", ";
  }

  os << "Dwell Area: " << value.dwell_area << ", ";

  if( value.sensor_orientation )
  {
    os << "Sensor Orientation: " << *value.sensor_orientation << ", ";
  }

  if( value.min_detectable_vel )
  {
    os << "Minimum Detectable Velocity: " << *value.min_detectable_vel
       << " dm/sec" << ", ";
  }

  os << "Target Reports: " << value.target_reports
     << " }";

  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP(
  stanag_4607_dwell_segment,
  &stanag_4607_dwell_segment::existence_mask,
  &stanag_4607_dwell_segment::revisit_index,
  &stanag_4607_dwell_segment::dwell_index,
  &stanag_4607_dwell_segment::last_dwell_of_revisit,
  &stanag_4607_dwell_segment::target_report_count,
  &stanag_4607_dwell_segment::dwell_time,
  &stanag_4607_dwell_segment::sensor_position,
  &stanag_4607_dwell_segment::scale_factor,
  &stanag_4607_dwell_segment::sensor_pos_uncert,
  &stanag_4607_dwell_segment::sensor_track,
  &stanag_4607_dwell_segment::sensor_speed,
  &stanag_4607_dwell_segment::sensor_vertical_vel,
  &stanag_4607_dwell_segment::sensor_track_uncert,
  &stanag_4607_dwell_segment::sensor_speed_uncert,
  &stanag_4607_dwell_segment::sensor_vertical_vel_uncert,
  &stanag_4607_dwell_segment::platform_orient,
  &stanag_4607_dwell_segment::dwell_area,
  &stanag_4607_dwell_segment::sensor_orientation,
  &stanag_4607_dwell_segment::min_detectable_vel,
  &stanag_4607_dwell_segment::target_reports
)

// ----------------------------------------------------------------------------
stanag_4607_dwell_segment_format
::stanag_4607_dwell_segment_format()
{}

// ----------------------------------------------------------------------------
stanag_4607_dwell_segment
stanag_4607_dwell_segment_format
::read( ptr_t& ptr ) const
{
  stanag_4607_dwell_segment result {};

  // Fields D1-D9 are mandatory
  auto mask = klv::klv_read_int< uint64_t >( ptr, (size_t)8 );
  mask >>= 16;
  result.existence_mask = klv::bitfield_to_enums<
      stanag_4607_dwell_existence_mask_bit, uint64_t >( mask );

  result.revisit_index = klv::klv_read_int< int >( ptr, (size_t)2 );

  result.dwell_index = klv::klv_read_int< int >( ptr, (size_t)2 );

  result.last_dwell_of_revisit = klv::klv_read_int< int >( ptr, (size_t)1 );

  result.target_report_count = klv::klv_read_int< int >( ptr, (size_t)2 );

  result.dwell_time = klv::klv_read_int< int >( ptr, (size_t)4 );

  result.sensor_position.latitude = klv::klv_read_flint< int32_t >({-90, 90},
                                                             ptr, (size_t)4);
  result.sensor_position.longitude =klv::klv_read_flint< uint32_t >(
                                           {0, 359.999999916}, ptr, (size_t)4);
  result.sensor_position.altitude = klv::klv_read_int< int >( ptr, (size_t)4 );

  // Fields D10-11 are conditional and always sent together
  // Condition: Sent if D32.4 and D32.5 are sent
  if( result.existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_SCALE_FACT_LAT) )
  {
    result.scale_factor = stanag_4607_scale_factor_format{}.read(ptr);
  }

  // Fields D12-D14 are optional and always sent together
  if( result.existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_POS_ALONG_TRACK) )
  {
    result.sensor_pos_uncert = stanag_4607_sensor_pos_uncert_format{}
                               .read(ptr);
  }

  // Fields D15-D17 are conditional and always sent together
  // Condition: Sent when the sensor system provides these parameters
  if( result.existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_TRACK) )
  {
    result.sensor_track = klv::klv_read_flint< uint16_t >({0, 359.9945},
                                                          ptr, 2);
    result.sensor_speed = klv::klv_read_int< int >( ptr, (size_t)4 );
    result.sensor_vertical_vel = klv::klv_read_int< int >( ptr, (size_t)1 );
  }

  // Fields D18-D20 are optional and always sent together
  if( result.existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_TRACK_UNCERT) )
  {
    result.sensor_track_uncert = klv::klv_read_int< int >( ptr, (size_t)1 );
    result.sensor_speed_uncert = klv::klv_read_int< int >( ptr, (size_t)2 );
    result.sensor_vertical_vel_uncert = klv::klv_read_int< int >( ptr,
                                                                  (size_t)2 );
  }

  // Fields D21-D23 are conditional and always sent together
  // Condition: Sent when the sensor system provides these parameters
  if( result.existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_PLATFORM_OREINT_HEADING) )
  {
    result.platform_orient = stanag_4607_orientation_format{}.read(ptr);
  }

  // Fields D24-27 are mandatory
  result.dwell_area.center_lat = klv::klv_read_flint< int32_t >({-90, 90},
                                                                ptr, 4);
  result.dwell_area.center_long = klv::klv_read_flint< uint32_t >(
                                                  {0, 359.999979}, ptr, 4);

  kwiver::vital::interval<double> interval = {0, 255.9928};
  size_t length = 2;
  auto v = klv::klv_read_int< int16_t >(ptr, length);
  auto const scale = interval.span() /
                     (( 0x80ull << ( ( length - 1 ) * 8 ) ) - 1);

  result.dwell_area.range_half_ext = v * scale + interval.lower();

  result.dwell_area.dwell_angle_half_ext = klv::klv_read_flint< uint16_t >(
                                                        {0, 359.9945}, ptr, 2);

  // Fields D28-D30 are optional
  // If at least one is sent, any omitted fields are set to 0
  if( result.existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_OREINT_HEADING) or
      result.existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_OREINT_PITCH) or
      result.existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_ORIENT_ROLL) )
  {
    result.sensor_orientation = stanag_4607_orientation_format{}.read(ptr);
  }

  // Field D31 is optional
  if( result.existence_mask.count(
      STANAG_4607_DWELL_EXIST_MASK_BIT_MIN_DETECT_VEL) )
  {
    result.min_detectable_vel = klv::klv_read_int< int >( ptr, (size_t)1 );
  }

  // Target reports
  std::vector < stanag_4607_target_report > target_reports;
  for(int i=0; i<result.target_report_count; i++)
  {
    stanag_4607_target_report_format target_report;
    stanag_4607_target_report target_report_data = target_report.read(
                                                  ptr, result.existence_mask);

    target_reports.push_back(target_report_data);
  }
  result.target_reports = target_reports;

  return result;
}

} // namespace stanag

} // namespace arrows

} // namespace kwiver
