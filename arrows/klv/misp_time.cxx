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

namespace {

// Indicates a functioning clock of unknown absolute-ness
constexpr uint8_t default_status = 0x9F;

} // namespace

// ----------------------------------------------------------------------------
misp_timestamp
::misp_timestamp()
  : timestamp{ 0 }, status{ default_status } {}

// ----------------------------------------------------------------------------
misp_timestamp
::misp_timestamp( uint64_t timestamp )
  : timestamp{ timestamp }, status{ default_status } {}

// ----------------------------------------------------------------------------
misp_timestamp
::misp_timestamp( uint64_t timestamp, uint8_t status )
  : timestamp{ timestamp }, status{ status } {}

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
misp_timestamp
read_misp_timestamp( klv_read_iter_t& data )
{
  // Skip tag to get to status and timestamp
  auto const is_nano =
    std::equal( misp_detail::tag_uuid_nano,
                misp_detail::tag_uuid_nano + misp_detail::tag_length,
                data );
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

  if( is_nano )
  {
    timestamp = ( timestamp + 500 ) / 1000;
  }

  return { timestamp, status };
}

// ----------------------------------------------------------------------------
void
write_misp_timestamp( misp_timestamp value, klv_write_iter_t& data,
                      misp_timestamp_tag_type tag_type )
{
  // Write tag
  auto const& tag =
    ( tag_type == MISP_TIMESTAMP_TAG_UUID )
    ? misp_detail::tag_uuid
    : misp_detail::tag_string;
  data = std::copy( tag, tag + misp_detail::tag_length, data );

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
size_t
misp_timestamp_length()
{
  return misp_detail::packet_length;
}

// ----------------------------------------------------------------------------
uint64_t
misp_timestamp_now()
{
  // TODO (C++20) Use std::chrono::tai_clock instead. Make sure to adjust for
  // TAI epoch being different than Unix epoch.

  // For now, we assume system_clock yields time since the Unix epoch,
  // including leap seconds, and estimate TAI from there.
  auto const time = std::chrono::system_clock::now().time_since_epoch();
  auto const microseconds_utc =
    std::chrono::duration_cast< std::chrono::microseconds >( time );

  // Hardcoding this is a hack, but AFAIK there is no portable way to get the
  // true current number of leap seconds without C++20.
  auto const leap_seconds = std::chrono::seconds( 27 );

  // UTC is 10 seconds behind TAI even without leap seconds.
  auto const tai_offset = std::chrono::seconds( 10 );
  auto const microseconds_tai = microseconds_utc + tai_offset + leap_seconds;

  // MISP time is (TAI since Unix epoch) - 8.000082 seconds.
  auto const misp_offset = std::chrono::microseconds( -8000082 );
  auto const microseconds_misp = microseconds_tai + misp_offset;

  return static_cast< uint64_t >( microseconds_misp.count() );
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
