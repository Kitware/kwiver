// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Defines a STANAG 4607 segment header and the various segment types

#ifndef KWIVER_ARROWS_STANAG_4607_SEGMENTS_H_
#define KWIVER_ARROWS_STANAG_4607_SEGMENTS_H_

#include "stanag_util.h"
#include <arrows/stanag/kwiver_algo_stanag_export.h>

#include <initializer_list>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace kwiver {

namespace arrows {

namespace stanag {

// ----------------------------------------------------------------------------
/// The type of message contained in the segment
enum KWIVER_ALGO_STANAG_EXPORT stanag_4607_segment_type : uint16_t
{
  STANAG_4607_SEGMENT_TYPE_MISSION                  = 1,
  STANAG_4607_SEGMENT_TYPE_DWELL                    = 2,
  STANAG_4607_SEGMENT_TYPE_HRR                      = 3,
  // Note: 4 is reserved
  STANAG_4607_SEGMENT_TYPE_JOB_DEFINITION           = 5,
  STANAG_4607_SEGMENT_TYPE_FREE_TEXT                = 6,
  STANAG_4607_SEGMENT_TYPE_LOW_REFLECTIVITY_INDEX   = 7,
  STANAG_4607_SEGMENT_TYPE_GROUP                    = 8,
  STANAG_4607_SEGMENT_TYPE_ATTACHED_TARGET          = 9,
  STANAG_4607_SEGMENT_TYPE_TEST_AND_STATUS          = 10,
  STANAG_4607_SEGMENT_TYPE_SYSTEM_SPECIFIC          = 11,
  STANAG_4607_SEGMENT_TYPE_PROCESSING_HISTORY       = 12,
  STANAG_4607_SEGMENT_TYPE_PLATFORM_LOCATION        = 13,
  // Note 14-100 are reserved for new segments
  STANAG_4607_SEGMENT_TYPE_JOB_REQUEST              = 101,
  STANAG_4607_SEGMENT_TYPE_JOB_ACKNOWLEDGE          = 102,
  // Note: 103-127 are reserved for future use
  // Note 128-255 are reserved for extensions
  STANAG_4607_SEGMENT_TYPE_ENUM_END                 = 256,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os,
            stanag_4607_segment_type const& value );

// ----------------------------------------------------------------------------
/// Identifies the type and size of the segment that follows
struct KWIVER_ALGO_STANAG_EXPORT stanag_4607_segment_header
{
  stanag_4607_segment_type segment_type;
  size_t segment_size;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os, stanag_4607_segment_header const& value );

// ----------------------------------------------------------------------------
DECLARE_STANAG_CMP( stanag_4607_segment_header )

// ----------------------------------------------------------------------------
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_segment_header_format
  : public stanag_4607_segment_header
{
public:
  stanag_4607_segment_header_format();


  const size_t size = 5; // Number of bytes in the segment header

  stanag_4607_segment_header
  read( ptr_t& ptr ) const;
};


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
/// Untyped base for STANAG formats
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_segment_data_format
{
public:
  explicit
  stanag_4607_segment_data_format() {}

  virtual
  ~stanag_4607_segment_data_format() = default;

  virtual stanag_4607_mission_segment // TODO: make this any segment type
  read( ptr_t& ptr ) const = 0;
};

using stanag_4607_segment_data_format_sptr =
  std::shared_ptr< stanag_4607_segment_data_format >;

// ----------------------------------------------------------------------------
/// Typed base for STANAG formats
template < class T >
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_segment_data_format_
  : public stanag_4607_segment_data_format
{
public:
  using data_type = T;

  explicit
  stanag_4607_segment_data_format_()
  {}

  virtual
  ~stanag_4607_segment_data_format_() {}

  virtual T
  read( ptr_t& ptr )  const = 0;
};

// ----------------------------------------------------------------------------
/// The type, enumeration, and name of a segment
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_segment_type_traits
{
public:
  stanag_4607_segment_type_traits( uint16_t type,
                                   std::string const& enum_name,
                                   stanag_4607_segment_data_format_sptr format,
                                   std::string const& name )
    : m_type{ type }, m_enum_name{ enum_name }, m_format{ format },
      m_name{ name }
  {}

  /// Returns the enumeration value of the segment
  uint16_t
  type() const { return m_type; }

  /// Return a string version of the segment enumeration
  std::string
  enum_name() const { return m_enum_name; }

  /// Return the data format used to represent this segment's value.
  stanag_4607_segment_data_format&
  format() const { return *m_format; }

  /// Return the segment's name.
  std::string
  name() const { return m_name; }

private:
  uint16_t m_type;
  std::string m_enum_name;
  stanag_4607_segment_data_format_sptr m_format;
  std::string m_name;
};

// ----------------------------------------------------------------------------
/// Lookup table used to match a segment to its traits
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_segment_type_traits_lookup
{
public:
  using iterator =
    typename std::vector< stanag_4607_segment_type_traits >::const_iterator;
  using init_list =
    typename std::initializer_list< stanag_4607_segment_type_traits >;

  stanag_4607_segment_type_traits_lookup(
    std::initializer_list< stanag_4607_segment_type_traits > const& traits );

  stanag_4607_segment_type_traits_lookup(
    std::vector< stanag_4607_segment_type_traits > const& traits );

  iterator
  begin() const { return m_traits.begin(); }

  iterator
  end() const { return m_traits.end(); }

  stanag_4607_segment_type_traits const&
  by_type( uint16_t type ) const;

  /// Return the traits object with \p enum_name as its enum name.
  stanag_4607_segment_type_traits const&
  by_enum_name( std::string const& enum_name ) const;

private:
  void initialize();


  std::vector< stanag_4607_segment_type_traits > m_traits;
  std::map< std::string,
            stanag_4607_segment_type_traits const* > m_enum_name_to_traits;
  std::map< uint16_t,
            stanag_4607_segment_type_traits const* > m_type_to_traits;
};

// ----------------------------------------------------------------------------
class KWIVER_ALGO_STANAG_EXPORT stanag_4607_mission_segment_format
  : public stanag_4607_segment_data_format_< stanag_4607_mission_segment >
{
public:
  stanag_4607_mission_segment_format();


  const size_t size = 39; // Number of bytes in mission segment

  stanag_4607_mission_segment
  read( ptr_t& ptr ) const override;
};

// ----------------------------------------------------------------------------
///
// TODO DWELL

// ----------------------------------------------------------------------------
/// Return a traits lookup object for segment types.
stanag_4607_segment_type_traits_lookup const&
stanag_4607_segment_type_traits_lookup_table();

} // namespace stanag

} // namespace arrows

} // namespace kwiver

#endif
