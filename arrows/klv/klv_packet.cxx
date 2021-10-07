// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV packet.

#include "klv_packet.h"

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

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

  // Read value
  auto const value =
    klv_lookup_packet_traits().by_uds_key( key )
    .format().read( data, length_of_value );

  return { key, value };
}

// ----------------------------------------------------------------------------
void
klv_write_packet( klv_packet const& packet, klv_write_iter_t& data,
                  size_t max_length )
{
  auto const& format =
    klv_lookup_packet_traits().by_uds_key( packet.key ).format();
  auto const length = format.length_of( packet.value );
  if( max_length < length )
  {
    VITAL_THROW( kwiver::vital::metadata_buffer_overflow,
                 "writing klv packet overflows buffer" );
  }
  format.write( packet.value, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_packet_length( klv_packet const& packet )
{
  return klv_lookup_packet_traits().by_uds_key( packet.key )
    .format().length_of( packet.value );
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
    { {},
      ENUM_AND_NAME( KLV_PACKET_MISB_1108_LOCAL_SET ),
      std::make_shared< klv_blob_format >(),
      "MISB ST 1108 Local Set",
      "Interpretability and Quality Local Set. Contains image quality metrics "
      "and compression characteristics for a video stream or file.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_PACKET_MISB_0601_LOCAL_SET ),
      std::make_shared< klv_blob_format >(),
      "MISB ST 0601 Local Set",
      "UAS Datalink Local Set. Contains a wide variety of metadata describing "
      "an unmanned aerial system producing FMV footage.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_PACKET_MISB_0104_UNIVERSAL_SET ),
      std::make_shared< klv_blob_format >(),
      "MISB ST 0104 Universal Set",
      "Predator UAV Basic Universal Set. Contains basic metadata describing a "
      "Predator unmanned aerial system producing FMV footage. Predecessor to "
      "MISB ST 0601. Deprecated as of 2008.",
      0 } };
#undef ENUM_AND_NAME

  return lookup;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
