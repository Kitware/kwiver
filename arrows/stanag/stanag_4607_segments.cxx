// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "stanag_4607_segments.h"

namespace ka = kwiver::arrows;

#include <vital/exceptions/base.h>

#include <ostream>
#include <initializer_list>
#include <vector>

namespace kwiver {

namespace arrows {

namespace stanag {

// ----------------------------------------------------------------------------
stanag_4607_segment_type_traits_lookup const&
stanag_4607_segment_type_traits_lookup_table()
{
  static stanag_4607_segment_type_traits_lookup const lookup = {
    {
      STANAG_4607_SEGMENT_TYPE_MISSION, "STANAG_4607_SEGMENT_TYPE_MISSION",
      std::make_shared< stanag_4607_mission_segment_format >(),
      "Mission Segment"
    }
  };

    // TODO finish rest of segments
    /*
    {
      STANAG_4607_SEGMENT_TYPE_DWELL,
      std::make_shared<>(),
      "Dwell Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_HRR,
      std::make_shared<>(),
      "HRR Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_JOB_DEFINITION,
      std::make_shared<>(),
      "Job Definition Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_FREE_TEXT,
      std::make_shared<>(),
      "Free Text Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_LOW_REFLECTIVITY_INDEX,
      std::make_shared<>(),
      "Low Reflectivity Index Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_GROUP,
      std::make_shared<>(),
      "Group Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_ATTACHED_TARGET,
      std::make_shared<>(),
      "Attached Target Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_TEST_AND_STATUS,
      std::make_shared<>(),
      "Test and Status Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_SYSTEM_SPECIFIC,
      std::make_shared<>(),
      "System-Specific Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_PROCESSING_HISTORY,
      std::make_shared<>(),
      "Processing History Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_PLATFORM_LOCATION,
      std::make_shared<>(),
      "Platform Location Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_JOB_REQUEST,
      std::make_shared<>(),
      "Job Request Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_JOB_ACKNOWLEDGE,
      std::make_shared<>(),
      "Job Acknowledge Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_ENUM_END,
      std::make_shared<>(),
      "Unknown STANAG4607 segment type"
    } */
  //};

  return lookup;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os,
            stanag_4607_segment_type const& value )
{
    os << stanag_4607_segment_type_traits_lookup_table().by_type( value )
          .name();

    return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, stanag_4607_segment_header const& value )
{
    os << "{ "
       << "Segment Type: " << value.segment_type << ", "
       << "Segment Size: " << value.segment_size
       << " }";

    return os;
}

// ----------------------------------------------------------------------------
DEFINE_STANAG_STRUCT_CMP(
  stanag_4607_segment_header,
  &stanag_4607_segment_header::segment_type,
  &stanag_4607_segment_header::segment_size
)

// ----------------------------------------------------------------------------
stanag_4607_segment_header_format
::stanag_4607_segment_header_format()
{}

// ----------------------------------------------------------------------------
stanag_4607_segment_header
stanag_4607_segment_header_format
::read( ptr_t& ptr ) const
{
    stanag_4607_segment_header result;

    result.segment_type = static_cast< stanag_4607_segment_type >(
        ka::klv::klv_read_int< uint64_t >( ptr, 1 ));
    result.segment_size = ka::klv::klv_read_int< size_t >( ptr, 4 );

    return result;
}

// ----------------------------------------------------------------------------
stanag_4607_segment_type_traits_lookup
::stanag_4607_segment_type_traits_lookup(
    std::initializer_list< stanag_4607_segment_type_traits > const& traits )
    : m_traits{ traits.begin(), traits.end() }
{
    initialize();
}

// ----------------------------------------------------------------------------
stanag_4607_segment_type_traits_lookup
::stanag_4607_segment_type_traits_lookup(
    std::vector< stanag_4607_segment_type_traits > const& traits )
    : m_traits{ traits.begin(), traits.end() }
{
    initialize();
}

// ----------------------------------------------------------------------------
stanag_4607_segment_type_traits const&
stanag_4607_segment_type_traits_lookup
::by_type( uint16_t type ) const
{
  auto const result = m_type_to_traits.find( type );
  return ( result == m_type_to_traits.end() )
        ? m_traits.at( 0 )
        : *result->second;
}

// ----------------------------------------------------------------------------
stanag_4607_segment_type_traits const&
stanag_4607_segment_type_traits_lookup
::by_enum_name( std::string const& enum_name ) const
{
    auto const result = m_enum_name_to_traits.find( enum_name );
    return ( result == m_enum_name_to_traits.end() )
         ? m_traits.at( 0 )
         : *result->second;
}

// ----------------------------------------------------------------------------
void
stanag_4607_segment_type_traits_lookup
::initialize()
{
  if( m_traits.empty() )
  {
    throw std::logic_error( "traits cannot be empty" );
  }

  for( auto const& trait : m_traits )
  {
    if( trait.type() )
    {
        m_type_to_traits.emplace( trait.type(), &trait ).second;
    }

    if( !trait.enum_name().empty() )
    {
        m_enum_name_to_traits.emplace( trait.enum_name(), &trait ).second;
    }
  }
}

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


} // namespace stanag

} // namespace arrows

} // namespace kwiver
