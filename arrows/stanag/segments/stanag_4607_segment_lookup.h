// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Defines a STANAG 4607 segment header and the various segment types

#ifndef KWIVER_ARROWS_STANAG_4607_SEGMENT_LOOKUP_H_
#define KWIVER_ARROWS_STANAG_4607_SEGMENT_LOOKUP_H_

#include <arrows/stanag/kwiver_algo_stanag_export.h>

#include "stanag_4607_dwell_segment.h"
#include "stanag_4607_mission_segment.h"

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
/// Return a traits lookup object for segment types.
stanag_4607_segment_type_traits_lookup const&
stanag_4607_segment_type_traits_lookup_table();

// ----------------------------------------------------------------------------
KWIVER_ALGO_STANAG_EXPORT
std::ostream&
operator<<( std::ostream& os,
            stanag_4607_segment_type const& value );

} // namespace stanag

} // namespace arrows

} // namespace kwiver

#endif
