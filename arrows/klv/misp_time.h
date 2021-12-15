// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Declare MISP timestamp utility functions.

// Code here based on the following standards:
// https://gwg.nga.mil/misb/docs/standards/ST0603.5.pdf
// https://gwg.nga.mil/misb/docs/standards/ST0604.3.pdf

#ifndef KWIVER_ARROWS_KLV_MISP_TIME_H
#define KWIVER_ARROWS_KLV_MISP_TIME_H

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/range/iota.h>

#include <algorithm>
#include <vector>

#include <cstdint>

namespace kwiver {

namespace arrows {

namespace klv {

namespace misp_detail {

constexpr ptrdiff_t tag_length = 16;
constexpr ptrdiff_t status_length = 1;
constexpr ptrdiff_t timestamp_length = 8 + 3;
constexpr ptrdiff_t packet_length =
  tag_length + status_length + timestamp_length;

} // namespace misp_detail

// ----------------------------------------------------------------------------
/// Locate a MISP timestamp packet in a sequence of bytes.
///
/// \param begin Iterator to beginning of byte sequence.
/// \param end Iterator to end of byte sequence.
///
/// \return Iterator to beginning of MISP packet, or \p end on failure.
template < class Iterator >
Iterator
find_misp_timestamp( Iterator begin, Iterator end )
{
  static auto const tag = std::string{ "MISPmicrosectime" };
  auto const it = std::search( begin, end, tag.cbegin(), tag.cend() );
  return ( std::distance( it, end ) < misp_detail::packet_length ) ? end : it;
}

// ----------------------------------------------------------------------------
/// Read a MISP timestamp from a sequence of bytes.
///
/// \param begin Iterator to beginning of MISP packet.
///
/// \return MISP microsecond timestamp.
template < class Iterator >
uint64_t
read_misp_timestamp( Iterator begin )
{
  // Skip tag and status to get to timestamp
  auto it = std::next( begin,
                       misp_detail::tag_length +
                       misp_detail::status_length );
  uint64_t result = 0;
  for( auto const i :
       kwiver::vital::range::iota( misp_detail::timestamp_length ) )
  {
    // Every third byte is set to 0xFF to avoid the timestamp being read as a
    // start tag for some other sort of data
    if( i % 3 != 2 )
    {
      result <<= 8;
      result |= *it;
    }
    ++it;
  }

  return result;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
