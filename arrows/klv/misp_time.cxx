// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Define MISP timestamp utility functions.

#include "misp_time.h"

#include <chrono>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
misp_timestamp
::misp_timestamp()
  : m_timestamp{ 0 }, m_status{ default_status }
{}

// ----------------------------------------------------------------------------
misp_timestamp
::misp_timestamp( std::chrono::microseconds timestamp, uint8_t status )
  : m_timestamp{ timestamp }, m_status{ status }
{}

// ----------------------------------------------------------------------------
misp_timestamp
::misp_timestamp( std::chrono::nanoseconds timestamp, uint8_t status )
  : m_timestamp{ timestamp }, m_status{ status }
{}

// ----------------------------------------------------------------------------
std::chrono::microseconds
misp_timestamp
::microseconds() const
{
  return std::chrono::microseconds{ ( m_timestamp.count() + 500 ) / 1000 };
}

// ----------------------------------------------------------------------------
void
misp_timestamp
::set_microseconds( std::chrono::microseconds microseconds )
{
  m_timestamp = std::chrono::nanoseconds{ microseconds };
}

// ----------------------------------------------------------------------------
std::chrono::nanoseconds
misp_timestamp
::nanoseconds() const
{
  return m_timestamp;
}

// ----------------------------------------------------------------------------
void
misp_timestamp
::set_nanoseconds( std::chrono::nanoseconds nanoseconds )
{
  m_timestamp = nanoseconds;
}

// ----------------------------------------------------------------------------
uint8_t
misp_timestamp
::status() const
{
  return m_status;
}

// ----------------------------------------------------------------------------
void
misp_timestamp
::set_status( uint8_t status )
{
  m_status = status;
}

// ----------------------------------------------------------------------------
klv_read_iter_t
find_misp_timestamp( klv_read_iter_t begin, klv_read_iter_t end,
                     misp_timestamp_tag_type tag_type )
{
  auto it = end;
  if( tag_type == MISP_TIMESTAMP_TAG_STRING )
  {
    auto const& tag = misp_detail::tag_string;
    it = std::search( begin, end, tag, tag + misp_detail::tag_length );
  }
  else if( tag_type == MISP_TIMESTAMP_TAG_UUID )
  {
    for( auto const& tag : { misp_detail::tag_uuid,
                             misp_detail::tag_uuid_nano } )
    {
      it = std::search( begin, end, tag, tag + misp_detail::tag_length );
      if( it != end )
      {
        break;
      }
    }
  }
  return ( std::distance( it, end ) < misp_detail::packet_length ) ? end : it;
}

// ----------------------------------------------------------------------------
bool
is_misp_timestamp_nano( klv_read_iter_t data )
{
  return std::equal(
    misp_detail::tag_uuid_nano,
    misp_detail::tag_uuid_nano + misp_detail::tag_length, data );
}

// ----------------------------------------------------------------------------
misp_timestamp
read_misp_timestamp( klv_read_iter_t& data )
{
  // Skip tag to get to status and timestamp
  auto const is_nano = is_misp_timestamp_nano( data );
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

  if( !is_nano )
  {
    timestamp *= 1000;
  }

  return { std::chrono::nanoseconds{ timestamp }, status };
}

// ----------------------------------------------------------------------------
void
write_misp_timestamp( misp_timestamp value, klv_write_iter_t& data,
                      misp_timestamp_tag_type tag_type, bool is_nano )
{
  // Write tag
  auto const& tag =
    ( tag_type == MISP_TIMESTAMP_TAG_UUID )
    ? ( is_nano ? misp_detail::tag_uuid_nano : misp_detail::tag_uuid )
    : misp_detail::tag_string;
  data = std::copy( tag, tag + misp_detail::tag_length, data );

  // Write status
  *data = value.status();
  ++data;

  auto timestamp =
    is_nano ? value.nanoseconds().count() : value.microseconds().count();
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
      *data = timestamp & mask;
      timestamp <<= 8;
    }
    ++data;
  }
}

// ----------------------------------------------------------------------------
size_t
misp_timestamp_length()
{
  return misp_detail::packet_length;
}

// ----------------------------------------------------------------------------
std::chrono::microseconds
misp_microseconds_now()
{
  return std::chrono::microseconds(
    ( misp_nanoseconds_now().count() + 500 ) / 1000 );
}

// ----------------------------------------------------------------------------
std::chrono::nanoseconds
misp_nanoseconds_now()
{
  // TODO (C++20) Use std::chrono::tai_clock instead. Make sure to adjust for
  // TAI epoch being different than Unix epoch.

  // For now, we assume system_clock yields time since the Unix epoch,
  // including leap seconds, and estimate TAI from there.
  auto const time = std::chrono::system_clock::now().time_since_epoch();
  auto const nanoseconds_utc =
    std::chrono::duration_cast< std::chrono::nanoseconds >( time );

  // Hardcoding this is a hack, but AFAIK there is no portable way to get the
  // true current number of leap seconds without C++20.
  auto const leap_seconds = std::chrono::seconds( 27 );

  // UTC is 10 seconds behind TAI even without leap seconds.
  auto const tai_offset = std::chrono::seconds( 10 );
  auto const nanoseconds_tai = nanoseconds_utc + tai_offset + leap_seconds;

  // MISP time is (TAI since Unix epoch) - 8.000082 seconds.
  auto const misp_offset = std::chrono::microseconds( -8000082 );
  auto const nanoseconds_misp = nanoseconds_tai + misp_offset;

  return nanoseconds_misp;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
