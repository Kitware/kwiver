// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "stanag_4607_mission_segment.h"

namespace ka = kwiver::arrows;

#include <vital/exceptions/base.h>

#include <ostream>
#include <initializer_list>
#include <vector>

namespace kwiver {

namespace arrows {

namespace stanag {


// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os,
            stanag_4607_mission_segment_platform const& value )
{
    std::map< stanag_4607_mission_segment_platform, std::string > strings
    {
        { STANAG_4607_MISSION_PLATFORM_UNIDENTIFIED, "Unidentified" },
        { STANAG_4607_MISSION_PLATFORM_ACS, "ACS" },
        { STANAG_4607_MISSION_PLATFORM_ARL_M, "ARL-M" },
        { STANAG_4607_MISSION_PLATFORM_SENTINEL, "Sentinel (was ASTOR)" },
        { STANAG_4607_MISSION_PLATFORM_ROTARY_WING_RADAR,
            "Rotary Wing Radar (was CRESO)" },
        { STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_NAVY, "Global Hawk-Navy" },
        { STANAG_4607_MISSION_PLATFORM_HORIZON, "HORIZON" },
        { STANAG_4607_MISSION_PLATFORM_E_8, "E-8 (Joint STARS)" },
        { STANAG_4607_MISSION_PLATFORM_P_3C, "P-3C" },
        { STANAG_4607_MISSION_PLATFORM_PREDATOR, "Predator" },
        { STANAG_4607_MISSION_PLATFORM_RADARSAT2, "RADARSAT2" },
        { STANAG_4607_MISSION_PLATFORM_U_2, "U-2" },
        { STANAG_4607_MISSION_PLATFORM_E_10, "E-10 (was MC2A)" },
        { STANAG_4607_MISSION_PLATFORM_UGS_SINGLE, "UGS – Single" },
        { STANAG_4607_MISSION_PLATFORM_UGS_CLUSTER, "UGS – Cluster" },
        { STANAG_4607_MISSION_PLATFORM_GROUND_BASED, "Ground Based" },
        { STANAG_4607_MISSION_PLATFORM_UAV_MARINES, "UAV-Marines" },
        { STANAG_4607_MISSION_PLATFORM_UAV_NAVY, "UAV-Navy" },
        { STANAG_4607_MISSION_PLATFORM_UAV_AIR_FORCE, "UAV-Air Force" },
        { STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_AIR_FORCE,
            "Global Hawk- Air Force" },
        { STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_AUSTRALIA,
            "Global Hawk-Australia" },
        { STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_GERMANY,
            "Global Hawk-Germany" },
        { STANAG_4607_MISSION_PLATFORM_PAUL_REVERE, "Paul Revere" },
        { STANAG_4607_MISSION_PLATFORM_MARINER_UAV, "Mariner UAV" },
        { STANAG_4607_MISSION_PLATFORM_BAC_11, "BAC-111" },
        { STANAG_4607_MISSION_PLATFORM_COYOTE, "Coyote" },
        { STANAG_4607_MISSION_PLATFORM_KING_AIR, "King Air" },
        { STANAG_4607_MISSION_PLATFORM_LIMIT, "LIMIT" },
        { STANAG_4607_MISSION_PLATFORM_NRL_NP_3B, "NRL NP-3B" },
        { STANAG_4607_MISSION_PLATFORM_SOSTAR_X, "SOSTAR-X" },
        { STANAG_4607_MISSION_PLATFORM_WATCHKEEPER, "WatchKeeper" },
        { STANAG_4607_MISSION_PLATFORM_ALLIANCE_GROUND_SURVEILLANCE,
            "Alliance Ground Surveillance (AGS) (A321)" },
        { STANAG_4607_MISSION_PLATFORM_STRYKER, "Stryker" },
        { STANAG_4607_MISSION_PLATFORM_AGS, "AGS (HALE UAV)" },
        { STANAG_4607_MISSION_PLATFORM_SIDM, "SIDM" },
        { STANAG_4607_MISSION_PLATFORM_REAPER, "Reaper" },
        { STANAG_4607_MISSION_PLATFORM_WARRIOR_A, "Warrior A" },
        { STANAG_4607_MISSION_PLATFORM_WARRIOR, "Warrior" },
        { STANAG_4607_MISSION_PLATFORM_TWIN_OTTER, "Twin Otter" },
        { STANAG_4607_MISSION_PLATFORM_OTHER, "Other" },
        { STANAG_4607_MISSION_PLATFORM_ENUM_END,
            "Unknown Mission Segment Platform Type" }
    };

    os << strings[ std::min( value, STANAG_4607_MISSION_PLATFORM_ENUM_END ) ];

    return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os,
            stanag_4607_mission_reference_time const& value )
{
    return os << "{ "
            << "Year: " << value.year << ", "
            << "Month: " << value.month << ", "
            << "Day: " << value.day
            << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP(
  stanag_4607_mission_reference_time,
  &stanag_4607_mission_reference_time::year,
  &stanag_4607_mission_reference_time::month,
  &stanag_4607_mission_reference_time::day
)

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_mission_segment const& value )
{
    return os << "{ "
              << "Mission Plan: " << value.mission_plan << ", "
              << "Flight Plan: " << value.flight_plan << ", "
              << "Platform Type: " << value.platform_type << ", "
              << "Platform Configuration: " << value.platform_configuration
              << ", "
              << "Reference Time: " << value.reference_time
              << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP(
  stanag_4607_mission_segment,
  &stanag_4607_mission_segment::mission_plan,
  &stanag_4607_mission_segment::flight_plan,
  &stanag_4607_mission_segment::platform_type,
  &stanag_4607_mission_segment::platform_configuration,
  &stanag_4607_mission_segment::reference_time
)

// ----------------------------------------------------------------------------
stanag_4607_mission_segment_format
::stanag_4607_mission_segment_format()
{}

// ----------------------------------------------------------------------------
stanag_4607_mission_segment
stanag_4607_mission_segment_format
::read( ptr_t& ptr ) const
{
  stanag_4607_mission_segment result;

  result.mission_plan = trim_whitespace( klv::klv_read_string( ptr,
                                                               (size_t)12 ) );

  result.flight_plan = trim_whitespace( klv::klv_read_string( ptr,
                                                              (size_t)12 ) );

  result.platform_type = static_cast< stanag_4607_mission_segment_platform >(
        klv::klv_read_int< uint64_t >( ptr, (size_t)1 ));

  result.platform_configuration = trim_whitespace( klv::klv_read_string(
                                                       ptr, (size_t)10 ) );

  result.reference_time.year = klv::klv_read_int< int >( ptr, (size_t)2 );
  result.reference_time.month = klv::klv_read_int< int >( ptr, (size_t)1 );
  result.reference_time.day = klv::klv_read_int< int >( ptr, (size_t)1 );

  return result;
}

} // namespace stanag

} // namespace arrows

} // namespace kwiver
