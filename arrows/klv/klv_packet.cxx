// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV packet.

#include "klv_packet.h"

#include <arrows/klv/klv_all.h>

#include <iomanip>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_packet
::klv_packet() : key{}, value{} {}

// ----------------------------------------------------------------------------
klv_packet
::klv_packet( klv_uds_key const& key, klv_value const& value )
  : key{ key },
    value{ value }
{}

// ----------------------------------------------------------------------------
klv_packet
::klv_packet( klv_uds_key const& key, klv_value&& value )
  : key{ key },
    value{ std::move( value ) }
{}

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
  auto const tracker = track_it( data, max_length );

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
  auto const key = klv_read_uds_key( data, tracker.remaining() );
  if( !key.is_valid() )
  {
    // This might be an encoding error, or maybe we falsely detected a prefix
    // in the data between the packets
    VITAL_THROW( kwiver::vital::metadata_exception, "invalid universal key" );
  }

  // Read length
  auto const length_of_value =
    klv_read_ber< size_t >( data, tracker.remaining() );
  if( max_length < tracker.remaining() )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "reading klv packet value overflows buffer" );
  }

  // Verify checksum
  auto const& format = klv_lookup_packet_traits().by_uds_key( key ).format();
  auto const checksum_format = format.checksum_format();
  size_t checksum_length = 0;
  if( checksum_format )
  {
    auto const packet_length = tracker.traversed() + length_of_value;
    checksum_length = checksum_format->fixed_length();

    auto it = tracker.begin() + packet_length - checksum_length;
    auto const expected_checksum =
      checksum_format->read_( it, checksum_length );
    auto const actual_checksum =
      checksum_format->evaluate(
        tracker.begin(),
        packet_length - checksum_length + checksum_format->header().size() );
    if( expected_checksum != actual_checksum )
    {
      LOG_ERROR(
        kv::get_logger( "klv" ),
        "calculated checksum "
        << "(" << checksum_format->to_string( actual_checksum ) << ") "
        << "does not equal checksum contained in packet "
        << "(" << checksum_format->to_string( expected_checksum ) << ")" );
    }
  }

  // Read value
  auto const value =
    format.read( data, length_of_value - checksum_length );

  // Ensure iterator ends correctly
  data += checksum_length;

  return { key, value };
}

// ----------------------------------------------------------------------------
void
klv_write_packet( klv_packet const& packet, klv_write_iter_t& data,
                  size_t max_length )
{
  auto const tracker = track_it( data, max_length );

  auto const& format =
    klv_lookup_packet_traits().by_uds_key( packet.key ).format();
  auto const checksum_format = format.checksum_format();
  auto const length = format.length_of( packet.value );
  auto const packet_length = klv_packet_length( packet );
  auto const checksum_length =
    checksum_format ? checksum_format->fixed_length() : 0;
  if( max_length < length + checksum_length )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "writing klv packet overflows buffer" );
  }

  klv_write_uds_key( packet.key, data, tracker.remaining() );
  klv_write_ber( length + checksum_length, data, tracker.remaining() );
  format.write( packet.value, data, length );

  if( checksum_format )
  {
    tracker.verify( checksum_length );
    auto const header = checksum_format->header();
    std::copy( header.begin(), header.end(), data );
    auto const checksum =
      checksum_format->evaluate(
        tracker.begin(),
        packet_length - checksum_length + header.size() );
    checksum_format->write_( checksum, data, checksum_length );
  }
}

// ----------------------------------------------------------------------------
size_t
klv_packet_length( klv_packet const& packet )
{
  auto const& format =
    klv_lookup_packet_traits().by_uds_key( packet.key ).format();
  auto const checksum_format = format.checksum_format();
  auto const length_of_key = packet.key.length;
  auto const length_of_value = format.length_of( packet.value );
  auto const length_of_length = klv_ber_length( length_of_value );
  auto const length_of_checksum =
    checksum_format ? checksum_format->fixed_length() : 0;
  return length_of_key + length_of_length + length_of_value +
         length_of_checksum;
}

// ----------------------------------------------------------------------------
kv::optional< uint64_t >
klv_packet_timestamp( klv_packet const& packet )
{
  if( !packet.value.valid() )
  {
    return kv::nullopt;
  }

  auto const get_local = [&]( klv_lds_key key ) -> kv::optional< uint64_t > {
    auto const& set = packet.value.get< klv_local_set >();
    auto const it = set.find( KLV_0601_PRECISION_TIMESTAMP );
    if ( it != set.end() && it->second.valid() )
    {
      return it->second.get< uint64_t >();
    }
    return kv::nullopt;
  };

  kv::optional< uint64_t > result;
  switch( klv_lookup_packet_traits().by_uds_key( packet.key ).tag() )
  {
    case KLV_PACKET_MISB_0104_UNIVERSAL_SET:
    {
      static auto const key = klv_0104_traits_lookup()
        .by_tag( KLV_0104_USER_DEFINED_TIMESTAMP ).uds_key();
      auto const& set = packet.value.get< klv_universal_set >();
      auto const it = set.find( key );
      if ( it != set.end() && it->second.valid() )
      {
        result = it->second.get< uint64_t >();
      }
      break;
    }
    case KLV_PACKET_MISB_0601_LOCAL_SET:
      return get_local( KLV_0601_PRECISION_TIMESTAMP );
    case KLV_PACKET_MISB_0806_LOCAL_SET:
      return get_local( KLV_0806_TIMESTAMP );
    case KLV_PACKET_MISB_0903_LOCAL_SET:
      return get_local( KLV_0903_PRECISION_TIMESTAMP );
    case KLV_PACKET_MISB_1002_LOCAL_SET:
      return get_local( KLV_1002_PRECISION_TIMESTAMP );
    case KLV_PACKET_MISB_1108_LOCAL_SET:
    {
      auto const& set = packet.value.get< klv_local_set >();
      auto const it = set.find( KLV_1108_METRIC_PERIOD_PACK );
      if ( it != set.end() && it->second.valid() )
      {
        result = it->second.get< klv_1108_metric_period_pack >().timestamp;
      }
      break;
    }
    case KLV_PACKET_UNKNOWN:
    default:
      break;
  }
  return result;
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_lookup_packet_traits()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_PACKET_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown Packet",
      "Packet of unknown type.",
      0 },
    { klv_0102_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_0102_LOCAL_SET ),
      std::make_shared< klv_0102_local_set_format >(),
      "MISB ST0102 Local Set",
      "Security Local Set. Used for marking Motion Imagery with security "
      "classification information.",
      0,
      &klv_0102_traits_lookup() },
    { klv_0104_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_0104_UNIVERSAL_SET ),
      std::make_shared< klv_0104_universal_set_format >(),
      "MISB ST0104 Universal Set",
      "Predator UAV Basic Universal Set. Contains basic metadata describing a "
      "Predator unmanned aerial system producing FMV footage. Predecessor to "
      "MISB ST 0601. Deprecated as of 2008.",
      0,
      &klv_0104_traits_lookup() },
    { klv_0601_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_0601_LOCAL_SET ),
      std::make_shared< klv_0601_local_set_format >(),
      "MISB ST0601 Local Set",
      "UAS Datalink Local Set. Contains a wide variety of metadata describing "
      "an unmanned aerial system producing FMV footage.",
      0,
      &klv_0601_traits_lookup() },
    { klv_0602_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_0602_UNIVERSAL_SET ),
      std::make_shared< klv_blob_format >(),
      "MISB ST0602 Universal Set",
      "Annotation Metadata Universal Set. Contains decriptions of visual cues "
      "meant to enhance the exploitation of the associated Motion Imagery.",
      0,
      nullptr },
    { klv_0806_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_0806_LOCAL_SET ),
      std::make_shared< klv_0806_local_set_format >(),
      "MISB ST0806 Local Set",
      "Remote Video Terminal Local Set. Contains metadata relating to the use "
      "of a Remote Video Terminal.",
      0,
      &klv_0806_traits_lookup() },
    { klv_0809_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_0809_LOCAL_SET ),
      std::make_shared< klv_blob_format >(),
      "MISB ST0809 Local Set",
      "Meteorological Metadata Local Set. Contains a broad range of basic "
      "information about atmospheric conditions.",
      0,
      nullptr },
    { klv_0903_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_0903_LOCAL_SET ),
      std::make_shared< klv_0903_local_set_format >(),
      "MISB ST0903 Local Set",
      "Video Moving Target Indicator Local Set. Contains information about "
      "objects detected in a Motion Imagery frame.",
      0,
      &klv_0903_traits_lookup() },
    { klv_1002_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_1002_LOCAL_SET ),
      std::make_shared< klv_1002_local_set_format >(),
      "MISB ST1002 Local Set",
      "Range Motion Imagery Local Set. Contains metadata particular to range "
      "imagery.",
      0,
      &klv_1002_traits_lookup() },
    { klv_1107_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_1107_LOCAL_SET ),
      std::make_shared< klv_blob_format >(),
      "MISB ST1107 Local Set",
      "Metric Geopositioning Metadata Local Set. Contains metadata relevant "
      "for photogrammetric applications.",
      0,
      nullptr },
    { klv_1108_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_1108_LOCAL_SET ),
      std::make_shared< klv_1108_local_set_format >(),
      "MISB ST1108 Local Set",
      "Interpretability and Quality Local Set. Contains image quality metrics "
      "and compression characteristics for a video stream or file.",
      0,
      &klv_1108_traits_lookup() },
    { klv_1202_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_1202_LOCAL_SET ),
      std::make_shared< klv_1202_local_set_format >(),
      "MISB ST1202 Local Set",
      "Generalized Transformation Local Set. Contains parameters describing a "
      "transformation from one two-dimensional coordinate system to another.",
      0,
      &klv_1202_traits_lookup() },
    { klv_1204_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_1204_MIIS_ID ),
      std::make_shared< klv_1204_miis_id_format >(),
      "MISB ST1204 MIIS ID",
      "Motion Imagery Identification System Core Identifier. Contains a "
      "unique identifier for the accompanying Motion Imagery.",
      0 },
    { klv_1206_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_1206_LOCAL_SET ),
      std::make_shared< klv_1206_local_set_format >(),
      "MISB ST1206 Local Set",
      "Synthetic Aperture Radar Motion Imagery Local Set. Contains metadata "
      "particular to SAR imagery.",
      0,
      &klv_1206_traits_lookup() },
    { klv_1507_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_1507_LOCAL_SET ),
      std::make_shared< klv_blob_format >(),
      "MISB ST1507 Local Set",
      "Sensor Timing Local Set. Contains information about the timing of the "
      "sensor shutter.",
      0,
      nullptr },
    { klv_1601_key(),
      ENUM_AND_NAME( KLV_PACKET_MISB_1601_LOCAL_SET ),
      std::make_shared< klv_1601_local_set_format >(),
      "MISB ST1601 Local Set",
      "Geo-Registration Local Set. Contains metadata concerning the process "
      "of mathematically revising sensor metadata, often through comparison "
      "with another image.",
      0,
      &klv_1601_traits_lookup() } };

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
