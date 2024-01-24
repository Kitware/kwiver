// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Definition of the KLV packet.

#ifndef KWIVER_ARROWS_KLV_KLV_PARSER_H_
#define KWIVER_ARROWS_KLV_KLV_PARSER_H_

#include <arrows/klv/klv_data_format.h>
#include <arrows/klv/klv_key.h>
#include <arrows/klv/klv_tag_traits.h>
#include <arrows/klv/klv_value.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/types/timestamp.h>

#include <optional>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
// Does not appear anywhere in KLV standards, just a way to consistently access
// traits of a specific standard
enum klv_top_level_tag : klv_lds_key
{
  KLV_PACKET_UNKNOWN,
  KLV_PACKET_MISB_0102_LOCAL_SET,
  KLV_PACKET_MISB_0104_UNIVERSAL_SET,
  KLV_PACKET_MISB_0601_LOCAL_SET,
  KLV_PACKET_MISB_0602_UNIVERSAL_SET,
  KLV_PACKET_MISB_0806_LOCAL_SET,
  KLV_PACKET_MISB_0809_LOCAL_SET,
  KLV_PACKET_MISB_0903_LOCAL_SET,
  KLV_PACKET_MISB_1002_LOCAL_SET,
  KLV_PACKET_MISB_1107_LOCAL_SET,
  KLV_PACKET_MISB_1108_LOCAL_SET,
  KLV_PACKET_MISB_1202_LOCAL_SET,
  KLV_PACKET_MISB_1204_MIIS_ID,
  KLV_PACKET_MISB_1206_LOCAL_SET,
  KLV_PACKET_MISB_1507_LOCAL_SET,
  KLV_PACKET_MISB_1601_LOCAL_SET,
  KLV_PACKET_ENUM_END,
};

// ----------------------------------------------------------------------------
/// Top-level KLV packet.
///
/// A KLV metadata stream consists of a sequence of these.
struct KWIVER_ALGO_KLV_EXPORT klv_packet
{
  klv_packet();

  klv_packet( klv_uds_key const& key, klv_value const& value );

  klv_packet( klv_uds_key const& key, klv_value&& value );

  klv_uds_key key;
  klv_value value;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator<( klv_packet const& lhs, klv_packet const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator==( klv_packet const& lhs, klv_packet const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator!=( klv_packet const& lhs, klv_packet const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_packet const& packet );

// ----------------------------------------------------------------------------
/// Find and read a KLV packet from a sequence of bytes.
///
/// This function will search for a valid UDS key in the given bytes,
/// indicating the beginning of a packet. It will skip any number of non-packet
/// bytes to find the next packet. The value
///
/// \param[in,out] data Iterator to sequence of \c uint8_t. Set to end of read
/// bytes on success or if no KLV is found; left as is on error.
/// \param max_length Maximum number of bytes to read.
///
/// \returns Packet read in from \p data.
///
/// \throws metadata_buffer_overflow If more bytes are needed to find or read
/// the next packet.
/// \throws metadata_exception If the UDS key prefix is found, but it is
/// attached to an invalid key.
KWIVER_ALGO_KLV_EXPORT
klv_packet
klv_read_packet( klv_read_iter_t& data, size_t max_length );

// ----------------------------------------------------------------------------
/// Write a KLV packet to a sequence of bytes.
///
/// \param packet KLV packet to write.
/// \param[in,out] data Writeable iterator to sequence of \c uint8_t. Set to
/// end of written bytes on success, left as is on error.
/// \param max_length Maximum number of bytes to write.
///
/// \throws metadata_buffer_overflow When \p packet is too large to fit in \p
/// length bytes.
KWIVER_ALGO_KLV_EXPORT
void
klv_write_packet( klv_packet const& packet,
                  klv_write_iter_t& data, size_t max_length );

// ----------------------------------------------------------------------------
/// Return the number of bytes required to store the given \p packet.
///
/// \param packet KLV packet whose byte length is being queried.
///
/// \returns Bytes required to store \p packet.
KWIVER_ALGO_KLV_EXPORT
size_t
klv_packet_length( klv_packet const& packet );

// ----------------------------------------------------------------------------
/// Return the time \p packet takes effect.
///
/// \param packet KLV packet being queried.
///
/// \returns Packet timestamp in microseconds, or \c nullopt on failure.
KWIVER_ALGO_KLV_EXPORT
std::optional< uint64_t >
klv_packet_timestamp( klv_packet const& packet );

// ----------------------------------------------------------------------------
/// Return a traits lookup object for top-level keys.
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_lookup_packet_traits();

// ----------------------------------------------------------------------------
struct KWIVER_ALGO_KLV_EXPORT klv_timed_packet
{
  klv_packet packet;
  kwiver::vital::timestamp timestamp;
  int stream_index;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator<( klv_timed_packet const& lhs, klv_timed_packet const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator==( klv_timed_packet const& lhs, klv_timed_packet const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator!=( klv_timed_packet const& lhs, klv_timed_packet const& rhs );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_timed_packet const& timed_packet );

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
