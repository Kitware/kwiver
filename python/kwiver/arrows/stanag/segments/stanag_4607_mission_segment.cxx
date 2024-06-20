// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "stanag_4607_mission_segment.h"

using ptr_t = uint8_t const*;

void
stanag_4607_mission_segment( py::module& m )
{
  py::enum_< kas::stanag_4607_mission_segment_platform >( m,
                                                          "stanag_4607_mission_segment_platform" )
    .value( "STANAG_4607_MISSION_PLATFORM_UNIDENTIFIED",
            kas::STANAG_4607_MISSION_PLATFORM_UNIDENTIFIED )
    .value( "STANAG_4607_MISSION_PLATFORM_ACS",
            kas::STANAG_4607_MISSION_PLATFORM_ACS )
    .value( "STANAG_4607_MISSION_PLATFORM_ARL_M",
            kas::STANAG_4607_MISSION_PLATFORM_ARL_M )
    .value( "STANAG_4607_MISSION_PLATFORM_SENTINEL",
            kas::STANAG_4607_MISSION_PLATFORM_SENTINEL )
    .value( "STANAG_4607_MISSION_PLATFORM_ROTARY_WING_RADAR",
            kas::STANAG_4607_MISSION_PLATFORM_ROTARY_WING_RADAR )
    .value( "STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_NAVY",
            kas::STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_NAVY )
    .value( "STANAG_4607_MISSION_PLATFORM_HORIZON",
            kas::STANAG_4607_MISSION_PLATFORM_HORIZON )
    .value( "STANAG_4607_MISSION_PLATFORM_E_8",
            kas::STANAG_4607_MISSION_PLATFORM_E_8 )
    .value( "STANAG_4607_MISSION_PLATFORM_P_3C",
            kas::STANAG_4607_MISSION_PLATFORM_P_3C )
    .value( "STANAG_4607_MISSION_PLATFORM_PREDATOR",
            kas::STANAG_4607_MISSION_PLATFORM_PREDATOR )
    .value( "STANAG_4607_MISSION_PLATFORM_RADARSAT2",
            kas::STANAG_4607_MISSION_PLATFORM_RADARSAT2 )
    .value( "STANAG_4607_MISSION_PLATFORM_U_2",
            kas::STANAG_4607_MISSION_PLATFORM_U_2 )
    .value( "STANAG_4607_MISSION_PLATFORM_E_10",
            kas::STANAG_4607_MISSION_PLATFORM_E_10 )
    .value( "STANAG_4607_MISSION_PLATFORM_UGS_SINGLE",
            kas::STANAG_4607_MISSION_PLATFORM_UGS_SINGLE )
    .value( "STANAG_4607_MISSION_PLATFORM_UGS_CLUSTER",
            kas::STANAG_4607_MISSION_PLATFORM_UGS_CLUSTER )
    .value( "STANAG_4607_MISSION_PLATFORM_GROUND_BASED",
            kas::STANAG_4607_MISSION_PLATFORM_GROUND_BASED )
    .value( "STANAG_4607_MISSION_PLATFORM_UAV_MARINES",
            kas::STANAG_4607_MISSION_PLATFORM_UAV_MARINES )
    .value( "STANAG_4607_MISSION_PLATFORM_UAV_NAVY",
            kas::STANAG_4607_MISSION_PLATFORM_UAV_NAVY )
    .value( "STANAG_4607_MISSION_PLATFORM_UAV_AIR_FORCE",
            kas::STANAG_4607_MISSION_PLATFORM_UAV_AIR_FORCE )
    .value( "STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_AIR_FORCE",
            kas::STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_AIR_FORCE )
    .value( "STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_AUSTRALIA",
            kas::STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_AUSTRALIA )
    .value( "STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_GERMANY",
            kas::STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_GERMANY )
    .value( "STANAG_4607_MISSION_PLATFORM_PAUL_REVERE",
            kas::STANAG_4607_MISSION_PLATFORM_PAUL_REVERE )
    .value( "STANAG_4607_MISSION_PLATFORM_MARINER_UAV",
            kas::STANAG_4607_MISSION_PLATFORM_MARINER_UAV )
    .value( "STANAG_4607_MISSION_PLATFORM_BAC_11",
            kas::STANAG_4607_MISSION_PLATFORM_BAC_11 )
    .value( "STANAG_4607_MISSION_PLATFORM_COYOTE",
            kas::STANAG_4607_MISSION_PLATFORM_COYOTE )
    .value( "STANAG_4607_MISSION_PLATFORM_KING_AIR",
            kas::STANAG_4607_MISSION_PLATFORM_KING_AIR )
    .value( "STANAG_4607_MISSION_PLATFORM_LIMIT",
            kas::STANAG_4607_MISSION_PLATFORM_LIMIT )
    .value( "STANAG_4607_MISSION_PLATFORM_NRL_NP_3B",
            kas::STANAG_4607_MISSION_PLATFORM_NRL_NP_3B )
    .value( "STANAG_4607_MISSION_PLATFORM_SOSTAR_X",
            kas::STANAG_4607_MISSION_PLATFORM_SOSTAR_X )
    .value( "STANAG_4607_MISSION_PLATFORM_WATCHKEEPER",
            kas::STANAG_4607_MISSION_PLATFORM_WATCHKEEPER )
    .value( "STANAG_4607_MISSION_PLATFORM_ALLIANCE_GROUND_SURVEILLANCE",
            kas::STANAG_4607_MISSION_PLATFORM_ALLIANCE_GROUND_SURVEILLANCE )
    .value( "STANAG_4607_MISSION_PLATFORM_STRYKER",
            kas::STANAG_4607_MISSION_PLATFORM_STRYKER )
    .value( "STANAG_4607_MISSION_PLATFORM_AGS",
            kas::STANAG_4607_MISSION_PLATFORM_AGS )
    .value( "STANAG_4607_MISSION_PLATFORM_SIDM",
            kas::STANAG_4607_MISSION_PLATFORM_SIDM )
    .value( "STANAG_4607_MISSION_PLATFORM_REAPER",
            kas::STANAG_4607_MISSION_PLATFORM_REAPER )
    .value( "STANAG_4607_MISSION_PLATFORM_WARRIOR_A",
            kas::STANAG_4607_MISSION_PLATFORM_WARRIOR_A )
    .value( "STANAG_4607_MISSION_PLATFORM_WARRIOR",
            kas::STANAG_4607_MISSION_PLATFORM_WARRIOR )
    .value( "STANAG_4607_MISSION_PLATFORM_TWIN_OTTER",
            kas::STANAG_4607_MISSION_PLATFORM_TWIN_OTTER )
    .value( "STANAG_4607_MISSION_PLATFORM_OTHER",
            kas::STANAG_4607_MISSION_PLATFORM_OTHER )
    .value( "STANAG_4607_MISSION_PLATFORM_ENUM_END",
            kas::STANAG_4607_MISSION_PLATFORM_ENUM_END )
    .def( "__str__",
          []( kas::stanag_4607_mission_segment_platform const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_mission_reference_time >( m,
                                                         "stanag_4607_mission_reference_time" )
    .def( py::init(
            []( int year, int month, int day ){
              return kas::stanag_4607_mission_reference_time{ year, month,
                                                              day };
            } )
          )
    .def_readwrite( "year", &kas::stanag_4607_mission_reference_time::year )
    .def_readwrite( "month", &kas::stanag_4607_mission_reference_time::month )
    .def_readwrite( "day", &kas::stanag_4607_mission_reference_time::day )
    .def( "__str__",
          []( kas::stanag_4607_mission_reference_time const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_mission_segment >( m,
                                                  "stanag_4607_mission_segment" )
    .def( py::init(
            []( std::string mission_plan, std::string flight_plan,
                kas::stanag_4607_mission_segment_platform platform_type,
                std::string platform_configuration,
                kas::stanag_4607_mission_reference_time reference_time ){
              return kas::stanag_4607_mission_segment{ mission_plan,
                                                       flight_plan,
                                                       platform_type,
                                                       platform_configuration,
                                                       reference_time };
            } ) )
    .def_readwrite( "mission_plan",
                    &kas::stanag_4607_mission_segment::mission_plan )
    .def_readwrite( "flight_plan",
                    &kas::stanag_4607_mission_segment::flight_plan )
    .def_readwrite( "platform_type",
                    &kas::stanag_4607_mission_segment::platform_type )
    .def_readwrite( "platform_configuration",
                    &kas::stanag_4607_mission_segment::platform_configuration )
    .def_readwrite( "reference_time",
                    &kas::stanag_4607_mission_segment::reference_time )
    .def( "__str__",
          []( kas::stanag_4607_mission_segment const& value ){
            std::stringstream s;
            s << value;
            return s.str();
          } );

  py::class_< kas::stanag_4607_mission_segment_format >( m,
                                                         "stanag_4607_mission_segment_format" )
    .def( py::init<>() )
    .def( "read",
          []( ptr_t ptr ){
            return kas::stanag_4607_mission_segment_format{}.read( ptr );
          } );
}
