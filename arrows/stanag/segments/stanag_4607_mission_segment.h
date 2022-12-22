// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Defines a STANAG 4607 segment header and the various segment types

#ifndef KWIVER_ARROWS_STANAG_4607_MISSION_SEGMENT_H_
#define KWIVER_ARROWS_STANAG_4607_MISSION_SEGMENT_H_

#include "stanag_4607_segments.h"
#include <arrows/stanag/kwiver_algo_stanag_export.h>
#include <arrows/stanag/stanag_util.h>

#include <initializer_list>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace kwiver {

namespace arrows {

namespace stanag {

namespace kv = kwiver::vital;

// ----------------------------------------------------------------------------
/// Identifies the type of platform that originated the data
enum KWIVER_ALGO_STANAG_EXPORT stanag_4607_mission_segment_platform
{
  STANAG_4607_MISSION_PLATFORM_UNIDENTIFIED,
  STANAG_4607_MISSION_PLATFORM_ACS,
  STANAG_4607_MISSION_PLATFORM_ARL_M,
  STANAG_4607_MISSION_PLATFORM_SENTINEL,
  STANAG_4607_MISSION_PLATFORM_ROTARY_WING_RADAR,
  STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_NAVY,
  STANAG_4607_MISSION_PLATFORM_HORIZON,
  STANAG_4607_MISSION_PLATFORM_E_8,
  STANAG_4607_MISSION_PLATFORM_P_3C,
  STANAG_4607_MISSION_PLATFORM_PREDATOR,
  STANAG_4607_MISSION_PLATFORM_RADARSAT2,
  STANAG_4607_MISSION_PLATFORM_U_2,
  STANAG_4607_MISSION_PLATFORM_E_10,
  STANAG_4607_MISSION_PLATFORM_UGS_SINGLE,
  STANAG_4607_MISSION_PLATFORM_UGS_CLUSTER,
  STANAG_4607_MISSION_PLATFORM_GROUND_BASED,
  STANAG_4607_MISSION_PLATFORM_UAV_MARINES,
  STANAG_4607_MISSION_PLATFORM_UAV_NAVY,
  STANAG_4607_MISSION_PLATFORM_UAV_AIR_FORCE,
  STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_AIR_FORCE,
  STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_AUSTRALIA,
  STANAG_4607_MISSION_PLATFORM_GLOBAL_HAWK_GERMANY,
  STANAG_4607_MISSION_PLATFORM_PAUL_REVERE,
  STANAG_4607_MISSION_PLATFORM_MARINER_UAV,
  STANAG_4607_MISSION_PLATFORM_BAC_11,
  STANAG_4607_MISSION_PLATFORM_COYOTE,
  STANAG_4607_MISSION_PLATFORM_KING_AIR,
  STANAG_4607_MISSION_PLATFORM_LIMIT,
  STANAG_4607_MISSION_PLATFORM_NRL_NP_3B,
  STANAG_4607_MISSION_PLATFORM_SOSTAR_X,
  STANAG_4607_MISSION_PLATFORM_WATCHKEEPER,
  STANAG_4607_MISSION_PLATFORM_ALLIANCE_GROUND_SURVEILLANCE,
  STANAG_4607_MISSION_PLATFORM_STRYKER,
  STANAG_4607_MISSION_PLATFORM_AGS,
  STANAG_4607_MISSION_PLATFORM_SIDM,
  STANAG_4607_MISSION_PLATFORM_REAPER,
  STANAG_4607_MISSION_PLATFORM_WARRIOR_A,
  STANAG_4607_MISSION_PLATFORM_WARRIOR,
  STANAG_4607_MISSION_PLATFORM_TWIN_OTTER,
  // Note: 40-254 are available for future use
  STANAG_4607_MISSION_PLATFORM_OTHER                             = 255,
  STANAG_4607_MISSION_PLATFORM_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os,
            stanag_4607_mission_segment_platform const& value );

// ----------------------------------------------------------------------------
/// UTC time in which the mission originated
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_mission_reference_time
{
  int year;
  int month;
  int day;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os,
            stanag_4607_mission_reference_time const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_mission_reference_time )

// ----------------------------------------------------------------------------
/// Information concerning the mission
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_mission_segment
{
  std::string mission_plan;
  std::string flight_plan;
  stanag_4607_mission_segment_platform platform_type;
  std::string platform_configuration;
  stanag_4607_mission_reference_time reference_time;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_mission_segment const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_mission_segment )

// ----------------------------------------------------------------------------
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_mission_segment_format
  : public stanag_4607_segment_data_format_< stanag_4607_mission_segment >
{
public:
  stanag_4607_mission_segment_format();

  using data_type = stanag_4607_mission_segment;

  stanag_4607_mission_segment
  read( ptr_t& ptr ) const;
};

} // namespace stanag

} // namespace arrows

} // namespace kwiver

#endif
