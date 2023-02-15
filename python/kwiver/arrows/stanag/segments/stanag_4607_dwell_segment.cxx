// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "stanag_4607_dwell_segment.h"

using ptr_t = uint8_t const*;

namespace kv = kwiver::vital;

void
stanag_4607_dwell_segment( py::module& m )
{
  py::class_< kas::stanag_4607_sensor_position >( m,
                                                  "stanag_4607_sensor_position" )
    .def( py::init( []( float latitude, float longitude, int32_t altitude){
                      return kas::stanag_4607_sensor_position{ latitude,
                                                               longitude,
                                                               altitude };
                    } ) )
    .def_readwrite( "latitude", &kas::stanag_4607_sensor_position::latitude )
    .def_readwrite( "longitude", &kas::stanag_4607_sensor_position::longitude )
    .def_readwrite( "altitude", &kas::stanag_4607_sensor_position::altitude )
    .def( "__str__",
          []( kas::stanag_4607_sensor_position const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_scale_factor >( m, "stanag_4607_scale_factor" )
    .def( py::init( []( float lat_scale, float long_scale ){
                      return kas::stanag_4607_scale_factor{ lat_scale,
                                                            long_scale };
                    } ) )
    .def_readwrite( "lat_scale", &kas::stanag_4607_scale_factor::lat_scale )
    .def_readwrite( "long_scale", &kas::stanag_4607_scale_factor::long_scale )
    .def( "__str__",
          []( kas::stanag_4607_scale_factor const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_scale_factor_format >( m,
                                                      "stanag_4607_scale_factor_format" )
    .def( py::init<>() )
    .def( "read",
          []( ptr_t ptr ){
            return kas::stanag_4607_scale_factor_format{}.read( ptr );
          } );

  py::class_< kas::stanag_4607_sensor_pos_uncert >( m,
                                                    "stanag_4607_sensor_pos_uncert" )
    .def( py::init( []( uint32_t along_track, uint32_t cross_track, uint16_t altitude ){
                      return kas::stanag_4607_sensor_pos_uncert{ along_track,
                                                                 cross_track,
                                                                 altitude };
                    } ) )
    .def_readwrite( "along_track",
                    &kas::stanag_4607_sensor_pos_uncert::along_track )
    .def_readwrite( "cross_track",
                    &kas::stanag_4607_sensor_pos_uncert::cross_track )
    .def_readwrite( "altitude", &kas::stanag_4607_sensor_pos_uncert::altitude )
    .def( "__str__",
          []( kas::stanag_4607_sensor_pos_uncert const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_sensor_pos_uncert_format >( m,
                                                           "stanag_4607_sensor_pos_uncert_format" )
    .def( py::init<>() )
    .def( "read",
          []( ptr_t ptr ){
            return kas::stanag_4607_sensor_pos_uncert_format{}.read( ptr );
          } );

  py::class_< kas::stanag_4607_orientation >( m, "stanag_4607_orientation" )
    .def( py::init( []( double heading, double pitch, double roll ){
                      return kas::stanag_4607_orientation{ heading, pitch,
                                                           roll };
                    } ) )
    .def_readwrite( "heading", &kas::stanag_4607_orientation::heading )
    .def_readwrite( "pitch", &kas::stanag_4607_orientation::pitch )
    .def_readwrite( "roll", &kas::stanag_4607_orientation::roll )
    .def( "__str__",
          []( kas::stanag_4607_orientation const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_orientation_format >( m,
                                                     "stanag_4607_orientation_format" )
    .def( py::init<>() )
    .def( "read",
          []( ptr_t ptr ){
            return kas::stanag_4607_orientation_format{}.read( ptr );
          } );

  py::class_< kas::stanag_4607_dwell_area >( m, "stanag_4607_dwell_area" )
    .def( py::init( []( float center_lat, float center_long,
                        float range_half_ext, float dwell_angle_half_ext ){
                      return kas::stanag_4607_dwell_area{ center_lat,
                                                          center_long,
                                                          range_half_ext,
                                                          dwell_angle_half_ext };
                    } ) )
    .def_readwrite( "center_lat", &kas::stanag_4607_dwell_area::center_lat )
    .def_readwrite( "center_long", &kas::stanag_4607_dwell_area::center_long )
    .def_readwrite( "range_half_ext",
                    &kas::stanag_4607_dwell_area::range_half_ext )
    .def_readwrite( "dwell_angle_half_ext",
                    &kas::stanag_4607_dwell_area::dwell_angle_half_ext )
    .def( "__str__",
          []( kas::stanag_4607_dwell_area const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::enum_< kas::stanag_4607_target_classification >( m,
                                                       "stanag_4607_target_classification" )
    .value( "STANAG_4607_TARGET_CLASS_NO_INFO_LIVE",
            kas::STANAG_4607_TARGET_CLASS_NO_INFO_LIVE )
    .value( "STANAG_4607_TARGET_CLASS_TRACKED_VEHICLE_LIVE",
            kas::STANAG_4607_TARGET_CLASS_TRACKED_VEHICLE_LIVE )
    .value( "STANAG_4607_TARGET_CLASS_WHEELED_VEHICLE_LIVE",
            kas::STANAG_4607_TARGET_CLASS_WHEELED_VEHICLE_LIVE )
    .value( "STANAG_4607_TARGET_CLASS_ROTARY_WING_AIRCRAFT_LIVE",
            kas::STANAG_4607_TARGET_CLASS_ROTARY_WING_AIRCRAFT_LIVE )
    .value( "STANAG_4607_TARGET_CLASS_FIXED_WING_AIRCRAFT_LIVE",
            kas::STANAG_4607_TARGET_CLASS_FIXED_WING_AIRCRAFT_LIVE )
    .value( "STANAG_4607_TARGET_CLASS_STATIONARY_ROTATOR_LIVE",
            kas::STANAG_4607_TARGET_CLASS_STATIONARY_ROTATOR_LIVE )
    .value( "STANAG_4607_TARGET_CLASS_MARITIME_LIVE",
            kas::STANAG_4607_TARGET_CLASS_MARITIME_LIVE )
    .value( "STANAG_4607_TARGET_CLASS_BEACON_LIVE",
            kas::STANAG_4607_TARGET_CLASS_BEACON_LIVE )
    .value( "STANAG_4607_TARGET_CLASS_AMPHIBIOUS_LIVE",
            kas::STANAG_4607_TARGET_CLASS_AMPHIBIOUS_LIVE )
    .value( "STANAG_4607_TARGET_CLASS_PERSON_LIVE",
            kas::STANAG_4607_TARGET_CLASS_PERSON_LIVE )
    .value( "STANAG_4607_TARGET_CLASS_VEHICLE_LIVE",
            kas::STANAG_4607_TARGET_CLASS_VEHICLE_LIVE )
    .value( "STANAG_4607_TARGET_CLASS_ANIMAL_LIVE",
            kas::STANAG_4607_TARGET_CLASS_ANIMAL_LIVE )
    .value( "STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_LIVE_LAND",
            kas::STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_LIVE_LAND )
    .value( "STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_LIVE_MARITIME",
            kas::STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_LIVE_MARITIME )
    .value( "STANAG_4607_TARGET_CLASS_OTHER_LIVE",
            kas::STANAG_4607_TARGET_CLASS_OTHER_LIVE )
    .value( "STANAG_4607_TARGET_CLASS_UNKNOWN_LIVE",
            kas::STANAG_4607_TARGET_CLASS_UNKNOWN_LIVE )
    .value( "STANAG_4607_TARGET_CLASS_NO_INFO_SIM",
            kas::STANAG_4607_TARGET_CLASS_NO_INFO_SIM )
    .value( "STANAG_4607_TARGET_CLASS_TRACKED_VEHICLE_SIM",
            kas::STANAG_4607_TARGET_CLASS_TRACKED_VEHICLE_SIM )
    .value( "STANAG_4607_TARGET_CLASS_WHEELED_VEHICLE_SIM",
            kas::STANAG_4607_TARGET_CLASS_WHEELED_VEHICLE_SIM )
    .value( "STANAG_4607_TARGET_CLASS_ROTARY_WING_AIRCRAFT_SIM",
            kas::STANAG_4607_TARGET_CLASS_ROTARY_WING_AIRCRAFT_SIM )
    .value( "STANAG_4607_TARGET_CLASS_FIXED_WING_AIRCRAFT_SIM",
            kas::STANAG_4607_TARGET_CLASS_FIXED_WING_AIRCRAFT_SIM )
    .value( "STANAG_4607_TARGET_CLASS_STATIONARY_ROTATOR_SIM",
            kas::STANAG_4607_TARGET_CLASS_STATIONARY_ROTATOR_SIM )
    .value( "STANAG_4607_TARGET_CLASS_MARITIME_SIM",
            kas::STANAG_4607_TARGET_CLASS_MARITIME_SIM )
    .value( "STANAG_4607_TARGET_CLASS_BEACON_SIM",
            kas::STANAG_4607_TARGET_CLASS_BEACON_SIM )
    .value( "STANAG_4607_TARGET_CLASS_AMPHIBIOUS_SIM",
            kas::STANAG_4607_TARGET_CLASS_AMPHIBIOUS_SIM )
    .value( "STANAG_4607_TARGET_CLASS_PERSON_SIM",
            kas::STANAG_4607_TARGET_CLASS_PERSON_SIM )
    .value( "STANAG_4607_TARGET_CLASS_VEHICLE_SIM",
            kas::STANAG_4607_TARGET_CLASS_VEHICLE_SIM )
    .value( "STANAG_4607_TARGET_CLASS_ANIMAL_SIM",
            kas::STANAG_4607_TARGET_CLASS_ANIMAL_SIM )
    .value( "STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_SIM_LAND",
            kas::STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_SIM_LAND )
    .value( "STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_SIM_MARITIME",
            kas::STANAG_4607_TARGET_CLASS_LARGE_MULTI_RETURN_SIM_MARITIME )
    .value( "STANAG_4607_TARGET_CLASS_TAGGING_DEVICE",
            kas::STANAG_4607_TARGET_CLASS_TAGGING_DEVICE )
    .value( "STANAG_4607_TARGET_CLASS_OTHER_SIM",
            kas::STANAG_4607_TARGET_CLASS_OTHER_SIM )
    .value( "STANAG_4607_TARGET_CLASS_UNKNOWN_SIM",
            kas::STANAG_4607_TARGET_CLASS_UNKNOWN_SIM )
    .value( "STANAG_4607_TARGET_CLASS_ENUM_END",
            kas::STANAG_4607_TARGET_CLASS_ENUM_END )
    .def( "__str__",
          []( kas::stanag_4607_target_classification const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_target_measure_uncert >( m,
                                                        "stanag_4607_target_measure_uncert" )
    .def( py::init( []( uint16_t slant_range, uint16_t cross_range, uint8_t height,
                        uint16_t radial_velocity ){
                      return kas::stanag_4607_target_measure_uncert{
                        slant_range, cross_range, height, radial_velocity };
                    } ) )
    .def_readwrite( "slant_range",
                    &kas::stanag_4607_target_measure_uncert::slant_range )
    .def_readwrite( "cross_range",
                    &kas::stanag_4607_target_measure_uncert::cross_range )
    .def_readwrite( "height", &kas::stanag_4607_target_measure_uncert::height )
    .def_readwrite( "radial_velocity",
                    &kas::stanag_4607_target_measure_uncert::radial_velocity )
    .def( "__str__",
          []( kas::stanag_4607_target_measure_uncert const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_target_measure_uncert_format >( m,
                                                               "stanag_4607_target_measure_uncert_format" )
    .def( py::init<>() )
    .def( "read",
          []( ptr_t ptr ){
            return kas::stanag_4607_target_measure_uncert_format{}.read( ptr );
          } );

  py::class_< kas::stanag_4607_truth_tag >( m, "stanag_4607_truth_tag" )
    .def( py::init( []( uint8_t application, uint32_t entity ){
                      return kas::stanag_4607_truth_tag{ application, entity };
                    } ) )
    .def_readwrite( "application", &kas::stanag_4607_truth_tag::application )
    .def_readwrite( "entity", &kas::stanag_4607_truth_tag::entity )
    .def( "__str__",
          []( kas::stanag_4607_truth_tag const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_truth_tag_format >( m,
                                                   "stanag_4607_truth_tag_format" )
    .def( py::init<>() )
    .def( "read",
          []( ptr_t ptr ){
            return kas::stanag_4607_truth_tag_format{}.read( ptr );
          } );

  py::enum_< kas::stanag_4607_dwell_existence_mask_bit >( m,
                                                          "stanag_4607_dwell_existence_mask_bit" )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_RADAR_CROSS_SECT",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_RADAR_CROSS_SECT )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_TRUTH_TAG_ENTITY",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_TRUTH_TAG_ENTITY )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_TRUTH_TAG_APPL",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_TRUTH_TAG_APPL )
    .value(
      "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_MEASURE_RADIAL_VEL",
      kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_MEASURE_RADIAL_VEL )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_MEASURE_HEIGHT",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_MEASURE_HEIGHT )
    .value(
      "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_MEASURE_CROSS_RNAGE",
      kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_MEASURE_CROSS_RNAGE )
    .value(
      "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_MEASURE_SLANT_RANGE",
      kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_MEASURE_SLANT_RANGE )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_CLASS_PROB",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_CLASS_PROB )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_CLASS",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_CLASS )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_SNR",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_SNR )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_WRAP_VEL",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_WRAP_VEL )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_VEL_LOS",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_VEL_LOS )
    .value(
      "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_GEODETIC_HEIGHT",
      kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_GEODETIC_HEIGHT )
    .value(
      "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_DELTA_LONG",
      kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_DELTA_LONG )
    .value(
      "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_DELTA_LAT",
      kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_DELTA_LAT )
    .value(
      "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_HI_RES_LONG",
      kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_HI_RES_LONG )
    .value(
      "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_HI_RES_LAT",
      kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_HI_RES_LAT )
    .value(
      "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_MTI_REPORT_IDX",
      kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_LOCATION_MTI_REPORT_IDX )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_MIN_DETECT_VEL",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_MIN_DETECT_VEL )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_ORIENT_ROLL",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_ORIENT_ROLL )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_OREINT_PITCH",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_OREINT_PITCH )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_OREINT_HEADING",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_OREINT_HEADING )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_AREA_DWELL_ANGLE_HALF",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_AREA_DWELL_ANGLE_HALF )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_AREA_RANGE_HALF",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_AREA_RANGE_HALF )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_AREA_CENTER_LONG",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_AREA_CENTER_LONG )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_AREA_CENTER_LAT",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_AREA_CENTER_LAT )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_PLATFORM_OREINT_ROLL",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_PLATFORM_OREINT_ROLL )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_PLATFORM_ORIENT_PITCH",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_PLATFORM_ORIENT_PITCH )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_PLATFORM_OREINT_HEADING",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_PLATFORM_OREINT_HEADING )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_VERTICAL_VEL_UNCERT",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_VERTICAL_VEL_UNCERT )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_SPEED_UNCERT",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_SPEED_UNCERT )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_TRACK_UNCERT",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_TRACK_UNCERT )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_VERTICAL_VEL",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_VERTICAL_VEL )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_SPEED",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_SPEED )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_TRACK",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_TRACK )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_POS_ALT",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_POS_ALT )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_POS_CROSS_TRACK",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_POS_CROSS_TRACK )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_POS_ALONG_TRACK",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_POS_ALONG_TRACK )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SCALE_FACT_LONG",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SCALE_FACT_LONG )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SCALE_FACT_LAT",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SCALE_FACT_LAT )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_ALT",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_ALT )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_LONG",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_LONG )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_LAT",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_SENSOR_LAT )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_TIME",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_TIME )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_COUNT",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_TARGET_REPORT_COUNT )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_LAST_DWELL_REVISIT",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_LAST_DWELL_REVISIT )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_INDEX",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_DWELL_INDEX )
    .value( "STANAG_4607_DWELL_EXIST_MASK_BIT_REVISIT_INDEX",
            kas::STANAG_4607_DWELL_EXIST_MASK_BIT_REVISIT_INDEX )
    .def( "__str__",
          []( kas::stanag_4607_dwell_existence_mask_bit const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_target_location >( m,
                                                  "stanag_4607_target_location" )
    .def( py::init( []( float hi_res_lat, float hi_res_long, int16_t delta_lat,
                        int16_t delta_long, int16_t geodetic_height ){
                      return kas::stanag_4607_target_location{ hi_res_lat,
                                                               hi_res_long,
                                                               delta_lat,
                                                               delta_long,
                                                               geodetic_height };
                    } ) )
    .def_readwrite( "hi_res_lat",
                    &kas::stanag_4607_target_location::hi_res_lat )
    .def_readwrite( "hi_res_long",
                    &kas::stanag_4607_target_location::hi_res_long )
    .def_readwrite( "delta_lat", &kas::stanag_4607_target_location::delta_lat )
    .def_readwrite( "delta_long",
                    &kas::stanag_4607_target_location::delta_long )
    .def_readwrite( "geodetic_height",
                    &kas::stanag_4607_target_location::geodetic_height )
    .def( "__str__",
          []( kas::stanag_4607_target_location const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_target_location_format >( m,
                                                         "stanag_4607_target_location_format" )
    .def( py::init<>() )
    .def( "read",
          []( ptr_t ptr,
              std::set< kas::stanag_4607_dwell_existence_mask_bit >
              existence_mask ){
            return kas::stanag_4607_target_location_format{}.read( ptr,
                                                                   existence_mask );
          } );

  py::class_< kas::stanag_4607_target_report >( m,
                                                "stanag_4607_target_report" )
    .def( py::init( [](std::optional< uint16_t > mti_report_idx,
                       std::optional< kas::stanag_4607_target_location >
                       location,
                       std::optional< int16_t > velocity_los,
                       std::optional< uint16_t > wrap_velocity,
                       std::optional< int8_t > snr,
                       std::optional< kas::stanag_4607_target_classification >
                       classification,
                       std::optional< uint8_t > class_probability,
                       std::optional< kas::stanag_4607_target_measure_uncert >
                       measurement_uncert,
                       std::optional< kas::stanag_4607_truth_tag > truth_tag,
                       std::optional< int8_t > radar_cross_sect ){
                      auto result = kas::stanag_4607_target_report{};

                      if( mti_report_idx.has_value() )
                      {
                        result.mti_report_idx = *mti_report_idx;
                      }
                      if( location.has_value() )
                      {
                        result.location = *location;
                      }
                      if( velocity_los.has_value() )
                      {
                        result.velocity_los = *velocity_los;
                      }
                      if( wrap_velocity.has_value() )
                      {
                        result.wrap_velocity = *wrap_velocity;
                      }
                      if( snr.has_value() ) { result.snr = *snr; }
                      if( classification.has_value() )
                      {
                        result.classification = *classification;
                      }
                      if( class_probability.has_value() )
                      {
                        result.class_probability = *class_probability;
                      }
                      if( measurement_uncert.has_value() )
                      {
                        result.measurement_uncert = *measurement_uncert;
                      }
                      if( truth_tag.has_value() )
                      {
                        result.truth_tag = *truth_tag;
                      }
                      if( radar_cross_sect.has_value() )
                      {
                        result.radar_cross_sect = *radar_cross_sect;
                      }

                      return result;
                    } ) )
    .def_readwrite( "mti_report_idx",
                    &kas::stanag_4607_target_report::mti_report_idx )
    .def_readwrite( "location", &kas::stanag_4607_target_report::location )
    .def_readwrite( "velocity_los",
                    &kas::stanag_4607_target_report::velocity_los )
    .def_readwrite( "wrap_velocity",
                    &kas::stanag_4607_target_report::wrap_velocity )
    .def_readwrite( "snr", &kas::stanag_4607_target_report::snr )
    .def_readwrite( "classification",
                    &kas::stanag_4607_target_report::classification )
    .def_readwrite( "class_probability",
                    &kas::stanag_4607_target_report::class_probability )
    .def_readwrite( "measurement_uncert",
                    &kas::stanag_4607_target_report::measurement_uncert )
    .def_readwrite( "truth_tag", &kas::stanag_4607_target_report::truth_tag )
    .def_readwrite( "radar_cross_sect",
                    &kas::stanag_4607_target_report::radar_cross_sect )
    .def( "__str__",
          []( kas::stanag_4607_target_report const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_target_report_format >( m,
                                                       "stanag_4607_target_report_format" )
    .def( py::init<>() )
    .def( "read",
          []( ptr_t ptr,
              std::set< kas::stanag_4607_dwell_existence_mask_bit >
              existence_mask ){
            return kas::stanag_4607_target_report_format{}.read( ptr,
                                                                 existence_mask );
          } );

  py::class_< kas::stanag_4607_dwell_segment >( m,
                                                "stanag_4607_dwell_segment" )
    .def( py::init( []( std::set< kas::stanag_4607_dwell_existence_mask_bit >
                        existence_mask,
                        uint16_t revisit_index, uint16_t dwell_index,
                        int last_dwell_of_revisit,
                        uint16_t target_report_count, uint32_t dwell_time,
                        kas::stanag_4607_sensor_position sensor_position,
                        std::optional< kas::stanag_4607_scale_factor >
                        scale_factor,
                        std::optional< kas::stanag_4607_sensor_pos_uncert >
                        sensor_pos_uncert,
                        std::optional< float > sensor_track,
                        std::optional< uint32_t > sensor_speed,
                        std::optional< int8_t > sensor_vertical_vel,
                        std::optional< uint8_t > sensor_track_uncert,
                        std::optional< uint16_t > sensor_speed_uncert,
                        std::optional< uint16_t > sensor_vertical_vel_uncert,
                        std::optional< kas::stanag_4607_orientation >
                        platform_orient,
                        kas::stanag_4607_dwell_area dwell_area,
                        std::optional< kas::stanag_4607_orientation >
                        sensor_orientation,
                        std::optional< uint8_t > min_detectable_vel,
                        std::vector< kas::stanag_4607_target_report >
                        target_reports ){
                      auto result = kas::stanag_4607_dwell_segment{};

                      // Mandatory fields
                      result.existence_mask = existence_mask;
                      result.revisit_index = revisit_index;
                      result.dwell_index = dwell_index;
                      result.last_dwell_of_revisit = last_dwell_of_revisit;
                      result.target_report_count = target_report_count;
                      result.dwell_time = dwell_time;
                      result.sensor_position = sensor_position;
                      result.dwell_area = dwell_area;
                      result.target_reports = target_reports;

                      // Optional/Conditional fields
                      if( scale_factor.has_value() )
                      {
                        result.scale_factor = *scale_factor;
                      }
                      if( sensor_pos_uncert.has_value() )
                      {
                        result.sensor_pos_uncert = *sensor_pos_uncert;
                      }
                      if( sensor_track.has_value() )
                      {
                        result.sensor_track = *sensor_track;
                      }
                      if( sensor_speed.has_value() )
                      {
                        result.sensor_speed = *sensor_speed;
                      }
                      if( sensor_vertical_vel.has_value() )
                      {
                        result.sensor_vertical_vel = *sensor_vertical_vel;
                      }
                      if( sensor_track_uncert.has_value() )
                      {
                        result.sensor_track_uncert = *sensor_track_uncert;
                      }
                      if( sensor_speed_uncert.has_value() )
                      {
                        result.sensor_speed_uncert = *sensor_speed_uncert;
                      }
                      if( sensor_vertical_vel_uncert.has_value() )
                      {
                        result.sensor_vertical_vel_uncert =
                          *sensor_vertical_vel_uncert;
                      }
                      if( platform_orient.has_value() )
                      {
                        result.platform_orient = *platform_orient;
                      }
                      if( sensor_orientation.has_value() )
                      {
                        result.sensor_orientation = *sensor_orientation;
                      }
                      if( min_detectable_vel.has_value() )
                      {
                        result.min_detectable_vel = *min_detectable_vel;
                      }

                      return result;
                    } ) )
    .def_readwrite( "existence_mask",
                    &kas::stanag_4607_dwell_segment::existence_mask )
    .def_readwrite( "revisit_index",
                    &kas::stanag_4607_dwell_segment::revisit_index )
    .def_readwrite( "dwell_index",
                    &kas::stanag_4607_dwell_segment::dwell_index )
    .def_readwrite( "last_dwell_of_revisit",
                    &kas::stanag_4607_dwell_segment::last_dwell_of_revisit )
    .def_readwrite( "target_report_count",
                    &kas::stanag_4607_dwell_segment::target_report_count )
    .def_readwrite( "dwell_time", &kas::stanag_4607_dwell_segment::dwell_time )
    .def_readwrite( "sensor_position",
                    &kas::stanag_4607_dwell_segment::sensor_position )
    .def_readwrite( "scale_factor",
                    &kas::stanag_4607_dwell_segment::scale_factor )
    .def_readwrite( "sensor_pos_uncert",
                    &kas::stanag_4607_dwell_segment::sensor_pos_uncert )
    .def_readwrite( "sensor_track",
                    &kas::stanag_4607_dwell_segment::sensor_track )
    .def_readwrite( "sensor_speed",
                    &kas::stanag_4607_dwell_segment::sensor_speed )
    .def_readwrite( "sensor_vertical_vel",
                    &kas::stanag_4607_dwell_segment::sensor_vertical_vel )
    .def_readwrite( "sensor_track_uncert",
                    &kas::stanag_4607_dwell_segment::sensor_track_uncert )
    .def_readwrite( "sensor_speed_uncert",
                    &kas::stanag_4607_dwell_segment::sensor_speed_uncert )
    .def_readwrite( "sensor_vertical_vel_uncert",
                    &kas::stanag_4607_dwell_segment::sensor_vertical_vel_uncert )
    .def_readwrite( "platform_orient",
                    &kas::stanag_4607_dwell_segment::platform_orient )
    .def_readwrite( "dwell_area", &kas::stanag_4607_dwell_segment::dwell_area )
    .def_readwrite( "sensor_orientation",
                    &kas::stanag_4607_dwell_segment::sensor_orientation )
    .def_readwrite( "min_detectable_vel",
                    &kas::stanag_4607_dwell_segment::min_detectable_vel )
    .def_readwrite( "target_reports",
                    &kas::stanag_4607_dwell_segment::target_reports )
    .def( "__str__",
          []( kas::stanag_4607_dwell_segment const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_dwell_segment_format >( m,
                                                       "stanag_4607_dwell_segment_format" )
    .def( py::init<>() )
    .def( "read",
          []( ptr_t ptr ){
            return kas::stanag_4607_dwell_segment_format{}.read( ptr );
          } );
}
