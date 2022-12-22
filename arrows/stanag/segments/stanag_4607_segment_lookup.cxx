// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

#include "stanag_4607_segment_lookup.h"

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
      STANAG_4607_SEGMENT_TYPE_MISSION,
      "STANAG_4607_SEGMENT_TYPE_MISSION",
      std::make_shared< stanag_4607_mission_segment_format >(),
      "Mission Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_DWELL,
      "STANAG_4607_SEGMENT_TYPE_DWELL",
      std::make_shared< stanag_4607_dwell_segment_format >(),
      "Dwell Segment"
    }

    // Currently supports a subset of the standard. More segments
    // will be added in the future.
    /*
    {
      STANAG_4607_SEGMENT_TYPE_HRR,
      "STANAG_4607_SEGMENT_TYPE_HRR",
      std::make_shared<>(),
      "HRR Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_JOB_DEFINITION,
      "STANAG_4607_SEGMENT_TYPE_JOB_DEFINITION",
      std::make_shared<>(),
      "Job Definition Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_FREE_TEXT,
      "STANAG_4607_SEGMENT_TYPE_FREE_TEXT",
      std::make_shared<>(),
      "Free Text Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_LOW_REFLECTIVITY_INDEX,
      "STANAG_4607_SEGMENT_TYPE_LOW_REFLECTIVITY_INDEX",
      std::make_shared<>(),
      "Low Reflectivity Index Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_GROUP,
      "STANAG_4607_SEGMENT_TYPE_GROUP",
      std::make_shared<>(),
      "Group Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_ATTACHED_TARGET,
      "STANAG_4607_SEGMENT_TYPE_ATTACHED_TARGET",
      std::make_shared<>(),
      "Attached Target Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_TEST_AND_STATUS,
      "STANAG_4607_SEGMENT_TYPE_TEST_AND_STATUS",
      std::make_shared<>(),
      "Test and Status Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_SYSTEM_SPECIFIC,
      "STANAG_4607_SEGMENT_TYPE_SYSTEM_SPECIFIC",
      std::make_shared<>(),
      "System-Specific Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_PROCESSING_HISTORY,
      "STANAG_4607_SEGMENT_TYPE_PROCESSING_HISTORY",
      std::make_shared<>(),
      "Processing History Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_PLATFORM_LOCATION,
      "STANAG_4607_SEGMENT_TYPE_PLATFORM_LOCATION",
      std::make_shared<>(),
      "Platform Location Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_JOB_REQUEST,
      "STANAG_4607_SEGMENT_TYPE_JOB_REQUEST",
      std::make_shared<>(),
      "Job Request Segment"
    },
    {
      STANAG_4607_SEGMENT_TYPE_JOB_ACKNOWLEDGE,
      "STANAG_4607_SEGMENT_TYPE_JOB_ACKNOWLEDGE",
      std::make_shared<>(),
      "Job Acknowledge Segment"
    },*/
    /*{
      STANAG_4607_SEGMENT_TYPE_ENUM_END,
      "STANAG_4607_SEGMENT_TYPE_ENUM_END",
      std::make_shared< void >(),
      "Unknown STANAG4607 segment type"
    } */
  };

  return lookup;
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
std::ostream&
operator<<( std::ostream& os,
            stanag_4607_segment_type const& value )
{
    os << stanag_4607_segment_type_traits_lookup_table().by_type( value )
          .name();

    return os;
}


} // namespace stanag

} // namespace arrows

} // namespace kwiver
