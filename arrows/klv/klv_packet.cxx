// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV packet.

#include "klv_packet.h"

#include <arrows/klv/klv_0104.h>
#include <arrows/klv/klv_0601.h>
#include <arrows/klv/klv_1108.h>

#include <iomanip>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
bool
operator<( klv_packet const& lhs, klv_packet const& rhs )
{
  if( lhs.key < rhs.key )
  {
    return true;
  }
  if( rhs.key < lhs.key )
  {
    return false;
  }
  return lhs.value < rhs.value;
}

// ----------------------------------------------------------------------------
bool
operator==( klv_packet const& lhs, klv_packet const& rhs )
{
  return lhs.key == rhs.key && lhs.value == rhs.value;
}

// ----------------------------------------------------------------------------
bool
operator!=( klv_packet const& lhs, klv_packet const& rhs )
{
  return !( lhs == rhs );
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_packet const& packet )
{
  auto const& traits = klv_lookup_packet_traits().by_uds_key( packet.key );
  os << "{ " << traits.name() << ": ";
  return traits.format().print( os, packet.value ) << " }";
}

// ----------------------------------------------------------------------------
klv_packet
klv_read_packet( klv_read_iter_t& data, size_t max_length )
{
  auto const begin = data;
  auto const remaining_length =
    [ & ]() -> size_t {
      return max_length - std::distance( begin, data );
    };

  // Find the prefix which begins all UDS keys
  auto const search_result =
    std::search( data, data + max_length,
                 klv_uds_key::prefix, klv_uds_key::prefix + 4 );
  if( search_result == data + max_length )
  {
    VITAL_THROW( kv::metadata_buffer_overflow,
                 "universal key not found in data buffer" );
  }

  // Sometimes encoders will put other data between KLV packets, so we may have
  // to skip some bytes
  if( search_result != data )
  {
    LOG_DEBUG( kv::get_logger( "klv" ), "skipped "
                << std::distance( data, search_result )
                << " bytes in klv stream" );
    data = search_result;
  }

  // Read key
  auto const key = klv_read_uds_key( data, remaining_length() );
  if( !key.is_valid() )
  {
    // This might be an encoding error, or maybe we falsely detected a prefix
    // in the data between the packets
    VITAL_THROW( kwiver::vital::metadata_exception, "invalid universal key" );
  }

  // Read length
  auto const length_of_value =
    klv_read_ber< size_t >( data, remaining_length() );
  if( max_length < remaining_length() )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "reading klv packet value overflows buffer" );
  }

  // Verify checksum
  auto const& format = klv_lookup_packet_traits().by_uds_key( key ).format();

  auto const packet_length =
    static_cast< size_t >( std::distance( begin, data ) ) + length_of_value;
  auto const checksum_length = format.checksum_length();

  auto const expected_checksum =
    format.read_checksum( begin, packet_length );
  auto const actual_checksum =
    format.calculate_checksum( begin, packet_length - checksum_length );
  if( expected_checksum != actual_checksum )
  {
    LOG_ERROR( kv::get_logger( "klv" ), std::hex << std::setfill( '0' )
               << "calculated checksum "
               << "(0x" << std::setw( 4 ) << actual_checksum << ") "
               << "does not equal checksum contained in packet "
               << "(0x" << std::setw( 4 ) << expected_checksum << ")" );
  }

  // Read value
  auto const value =
    format.read( data, length_of_value - checksum_length );

  // Ensure iterator ends correctly
  data += format.checksum_length();

  return { key, value };
}

// ----------------------------------------------------------------------------
void
klv_write_packet( klv_packet const& packet, klv_write_iter_t& data,
                  size_t max_length )
{
  auto const begin = data;
  auto const remaining_length =
    [ & ]() -> size_t {
      return max_length - std::distance( begin, data );
    };

  auto const& format =
    klv_lookup_packet_traits().by_uds_key( packet.key ).format();
  auto const length = format.length_of( packet.value );
  auto const packet_length = klv_packet_length( packet );
  auto const checksum_length = format.checksum_length();
  if( max_length < length + checksum_length )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "writing klv packet overflows buffer" );
  }

  klv_write_uds_key( packet.key, data, remaining_length() );
  klv_write_ber( length + checksum_length, data, remaining_length() );
  format.write( packet.value, data, length );

  auto const checksum =
    format.calculate_checksum( begin, packet_length - checksum_length );
  format.write_checksum( checksum, data, checksum_length );
}

// ----------------------------------------------------------------------------
size_t
klv_packet_length( klv_packet const& packet )
{
  auto const& format =
    klv_lookup_packet_traits().by_uds_key( packet.key ).format();
  auto const length_of_key = packet.key.length;
  auto const length_of_value = format.length_of( packet.value );
  auto const length_of_length = klv_ber_length( length_of_value );
  auto const length_of_checksum = format.checksum_length();
  return length_of_key + length_of_length + length_of_value +
         length_of_checksum;
}

// ----------------------------------------------------------------------------
uint64_t
klv_packet_timestamp( klv_packet const& packet )
{
  if( !packet.value.valid() )
  {
    return 0;
  }
  switch( klv_lookup_packet_traits().by_uds_key( packet.key ).tag() )
  {
    case KLV_PACKET_MISB_0104_UNIVERSAL_SET:
    {
      static auto const key = klv_0104_traits_lookup()
        .by_tag( KLV_0104_USER_DEFINED_TIMESTAMP ).uds_key();
      auto const& set = packet.value.get< klv_universal_set >();
      auto const it = set.find( key );
      return ( it != set.end() && it->second.valid() )
             ? it->second.get< uint64_t >() : 0;
    }
    case KLV_PACKET_MISB_0601_LOCAL_SET:
    {
      auto const& set = packet.value.get< klv_local_set >();
      auto const it = set.find( KLV_0601_PRECISION_TIMESTAMP );
      return ( it != set.end() && it->second.valid() )
             ? it->second.get< uint64_t >() : 0;
    }
    case KLV_PACKET_MISB_1108_LOCAL_SET:
    {
      auto const& set = packet.value.get< klv_local_set >();
      auto const it = set.find( KLV_1108_METRIC_PERIOD_PACK );
      return ( it != set.end() && it->second.valid() )
             ? it->second.get< klv_1108_metric_period_pack >().timestamp : 0;
    }
    case KLV_PACKET_UNKNOWN:
    default:
      return 0;
  }
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_lookup_packet_traits()
{
  // TODO: Edit these once parsers are finalized
#define ENUM_AND_NAME( X ) X, #X
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_PACKET_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown Packet",
      "Packet of unknown type.",
      0 },
    { klv_1108_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_1108_LOCAL_SET ),
      std::make_shared< klv_1108_local_set_format >(),
      "MISB ST 1108 Local Set",
      "Interpretability and Quality Local Set. Contains image quality metrics "
      "and compression characteristics for a video stream or file.",
      0,
      &klv_1108_traits_lookup() },
    { klv_0601_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_0601_LOCAL_SET ),
      std::make_shared< klv_0601_local_set_format >(),
      "MISB ST 0601 Local Set",
      "UAS Datalink Local Set. Contains a wide variety of metadata describing "
      "an unmanned aerial system producing FMV footage.",
      0,
      &klv_0601_traits_lookup() },
    { klv_0104_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_0104_UNIVERSAL_SET ),
      std::make_shared< klv_0104_universal_set_format >(),
      "MISB ST 0104 Universal Set",
      "Predator UAV Basic Universal Set. Contains basic metadata describing a "
      "Predator unmanned aerial system producing FMV footage. Predecessor to "
      "MISB ST 0601. Deprecated as of 2008.",
      0,
      &klv_0104_traits_lookup() } };
#undef ENUM_AND_NAME

  return lookup;
}

// ----------------------------------------------------------------------------
bool
operator<( klv_timed_packet const& lhs, klv_timed_packet const& rhs )
{
  if( lhs.timestamp.has_valid_time() && !rhs.timestamp.has_valid_time() )
  {
    return true;
  }
  if( !lhs.timestamp.has_valid_time() )
  {
    return false;
  }
  if( lhs.timestamp.get_time_usec() < rhs.timestamp.get_time_usec() )
  {
    return true;
  }
  if( lhs.timestamp.get_time_usec() > rhs.timestamp.get_time_usec() )
  {
    return false;
  }

  if( lhs.timestamp.has_valid_frame() && !rhs.timestamp.has_valid_frame() )
  {
    return true;
  }
  if( !lhs.timestamp.has_valid_frame() )
  {
    return false;
  }
  if( lhs.timestamp.get_frame() < rhs.timestamp.get_frame() )
  {
    return true;
  }
  if( lhs.timestamp.get_frame() > rhs.timestamp.get_frame() )
  {
    return false;
  }

  return lhs.packet < rhs.packet;
}

// ----------------------------------------------------------------------------
bool
operator==( klv_timed_packet const& lhs, klv_timed_packet const& rhs )
{
  return lhs.timestamp.has_valid_time() == rhs.timestamp.has_valid_time() &&
         lhs.timestamp.has_valid_frame() == rhs.timestamp.has_valid_frame() &&
         ( !lhs.timestamp.has_valid_time() ||
           lhs.timestamp.get_time_usec() == rhs.timestamp.get_time_usec() ) &&
         ( !lhs.timestamp.has_valid_frame() ||
           lhs.timestamp.get_frame() == rhs.timestamp.get_frame() ) &&
         lhs.packet == rhs.packet;
}

// ----------------------------------------------------------------------------
bool
operator!=( klv_timed_packet const& lhs, klv_timed_packet const& rhs )
{
  return !( lhs == rhs );
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_timed_packet const& timed_packet )
{
  return os << "{ timestamp: " << timed_packet.timestamp << ", "
            << "packet: " << timed_packet.packet << " }";
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
