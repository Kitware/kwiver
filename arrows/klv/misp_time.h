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
#include <string>
#include <vector>

#include <cstddef>
#include <cstdint>

namespace kwiver {

namespace arrows {

namespace klv {

namespace misp_detail {

constexpr std::ptrdiff_t tag_length = 16;
constexpr std::ptrdiff_t status_length = 1;
constexpr std::ptrdiff_t timestamp_length = 8 + 3;
constexpr std::ptrdiff_t packet_length =
  tag_length + status_length + timestamp_length;
auto const tag_bytes = std::string{ "MISPmicrosectime" };

} // namespace misp_detail

// ----------------------------------------------------------------------------
/// Bit indices for the MISP timestamp status.
enum misp_timestamp_status_bit
{
  // Bits 0-4 reserved for future use; should be set to 1 for now

  // 0 = jump forward in time, 1 = jump backward in time
  MISP_TIMESTAMP_STATUS_BIT_DISCONTINUITY_REVERSE = 5,

  // 0 = normal, 1 = time discontinuity (jump forward or backward)
  MISP_TIMESTAMP_STATUS_BIT_DISCONTINUITY = 6,

  // 0 = time is locked to absolute reference, 1 = time may not be locked
  MISP_TIMESTAMP_STATUS_BIT_NOT_LOCKED = 7,

  MISP_TIMESTAMP_STATUS_BIT_ENUM_END,
};

// ----------------------------------------------------------------------------
/// Frame timestamp information embedded in the video stream.
struct KWIVER_ALGO_KLV_EXPORT misp_timestamp
{
  misp_timestamp();

  misp_timestamp( uint64_t timestamp );

  misp_timestamp( uint64_t timestamp, uint8_t status );

  uint64_t timestamp;
  uint8_t status;
};

// ----------------------------------------------------------------------------
/// Locate a MISP microsecond timestamp packet in a sequence of bytes.
///
/// \param begin Iterator to beginning of byte sequence.
/// \param end Iterator to end of byte sequence.
///
/// \return Iterator to beginning of MISP packet, or \p end on failure.
template < class Iterator >
Iterator
find_misp_timestamp( Iterator begin, Iterator end )
{
  auto const& tag = misp_detail::tag_bytes;
  auto const it = std::search( begin, end, tag.cbegin(), tag.cend() );
  return ( std::distance( it, end ) < misp_detail::packet_length ) ? end : it;
}

// ----------------------------------------------------------------------------
/// Read a MISP timestamp from a sequence of bytes.
///
/// \param data Iterator to beginning of MISP packet. Set to end of read bytes
///             on success.
///
/// \return MISP microsecond timestamp.
template < class Iterator >
misp_timestamp
read_misp_timestamp( Iterator& data )
{
  // Skip tag to get to status and timestamp
  std::advance( data, misp_detail::tag_length );

  auto const status = *data;
  ++data;

  uint64_t timestamp = 0;
  for( auto const i :
       kwiver::vital::range::iota( misp_detail::timestamp_length ) )
  {
    // Every third byte is set to 0xFF to avoid the timestamp being read as a
    // start tag for some other sort of data
    if( i % 3 != 2 )
    {
      timestamp <<= 8;
      timestamp |= *data;
    }
    ++data;
  }

  return { timestamp, status };
}

// ----------------------------------------------------------------------------
/// Write a MISP timestamp to a sequence of bytes.
///
/// \param value Timestamp value to write.
/// \param data Iterator to sequence of \c uint8_t. Set to end of written bytes
//              on success.
template < class Iterator >
void
write_misp_timestamp( misp_timestamp value, Iterator& data )
{
  // Write tag
  auto const& tag = misp_detail::tag_bytes;
  data = std::copy( tag.begin(), tag.end(), data );

  // Write status
  *data = value.status;
  ++data;

  for( auto const i :
       kwiver::vital::range::iota( misp_detail::timestamp_length ) )
  {
    if( i % 3 == 2 )
    {
      // Every third byte is set to 0xFF to avoid the timestamp being read as a
      // start tag for some other sort of data
      *data = 0xFF;
    }
    else
    {
      // Write the next most significant byte
      constexpr uint64_t mask = 0xFF << 7;
      *data = value.timestamp & mask;
      value.timestamp <<= 8;
    }
    ++data;
  }
}

// ----------------------------------------------------------------------------
/// Return the length of a MISP timestamp packet in bytes.
KWIVER_ALGO_KLV_EXPORT
size_t
misp_timestamp_length();

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
