// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 1108 parser.

#include "klv_1108.h"

#include <arrows/klv/klv_1108_metric_set.h>
#include <arrows/klv/klv_checksum.h>

#include <algorithm>
#include <iomanip>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

// ----------------------------------------------------------------------------
constexpr size_t checksum_packet_length = 4;

} // namespace

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1108_assessment_point value )
{
  static std::string strings[ KLV_1108_ASSESSMENT_POINT_ENUM_END ] = {
    "Unknown Assessment Point",
    "Sensor",
    "Sensor Encoder",
    "GCS (Received)",
    "GCS (Transmitted)",
    "Library / Archive" };

  os << strings[ ( value < KLV_1108_ASSESSMENT_POINT_ENUM_END ) ? value : 0 ];
  return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1108_compression_type value )
{
  static std::string strings[ KLV_1108_COMPRESSION_TYPE_ENUM_END + 1 ] = {
    "Uncompressed",
    "H.262",
    "H.264",
    "H.265",
    "JPEG2000",
    "Unknown Compression Type" };

  os << strings[ std::min( value, KLV_1108_COMPRESSION_TYPE_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1108_compression_profile value )
{
  static std::string strings[ KLV_1108_COMPRESSION_PROFILE_ENUM_END + 1 ] = {
    "Uncompressed",
    "Main (H.264)",
    "Main 10 (H.265)",
    "Constrained Baseline (H.264)",
    "High (H.264)",
    "Main 4:2:2 12 (H.265)",
    "Main 4:4:4 12 (H.265)",
    "High 4:2:2 (H.264)",
    "High 4:4:4 Predictive (H.264)",
    "Unknown Compression Profile" };

  os << strings[ std::min( value, KLV_1108_COMPRESSION_PROFILE_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
bool
operator==( klv_1108_metric_period_pack const& lhs,
            klv_1108_metric_period_pack const& rhs )
{
  return lhs.timestamp == rhs.timestamp && lhs.offset == rhs.offset;
}

// ----------------------------------------------------------------------------
bool
operator<( klv_1108_metric_period_pack const& lhs,
           klv_1108_metric_period_pack const& rhs )
{
  if( lhs.timestamp < rhs.timestamp )
  {
    return true;
  }
  else if( rhs.timestamp < lhs.timestamp )
  {
    return false;
  }

  if( lhs.offset < rhs.offset )
  {
    return true;
  }
  else if( rhs.offset < lhs.offset )
  {
    return false;
  }

  return false;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1108_metric_period_pack const& rhs )
{
  return os << "{ Timestamp: " << rhs.timestamp << ", "
            << "Offset: " << rhs.offset << " }";
}

// ----------------------------------------------------------------------------
klv_1108_metric_period_pack_format
::klv_1108_metric_period_pack_format() : klv_data_format_< data_type >{ 12 }
{}

// ----------------------------------------------------------------------------
klv_1108_metric_period_pack
klv_1108_metric_period_pack_format
::read_typed( klv_read_iter_t& data, VITAL_UNUSED size_t length ) const
{
  auto const timestamp = klv_read_int< uint64_t >( data, 8 );
  auto const offset    = klv_read_int< uint32_t >( data, 4 );
  return { timestamp, offset };
}

// ----------------------------------------------------------------------------
void
klv_1108_metric_period_pack_format
::write_typed( klv_1108_metric_period_pack const& value,
               klv_write_iter_t& data, VITAL_UNUSED size_t length ) const
{
  klv_write_int( value.timestamp, data, 8 );
  klv_write_int( value.offset,    data, 4 );
}

// ----------------------------------------------------------------------------
std::string
klv_1108_metric_period_pack_format
::description() const
{
  return "metric period pack of " + length_description();
}

// ----------------------------------------------------------------------------
bool
operator==( klv_1108_window_corners_pack const& lhs,
            klv_1108_window_corners_pack const& rhs )
{
  return std::equal( &lhs.top_row, &lhs.top_row + 4, &rhs.top_row );
}

// ----------------------------------------------------------------------------
bool
operator<( klv_1108_window_corners_pack const& lhs,
           klv_1108_window_corners_pack const& rhs )
{
  return std::lexicographical_compare( &lhs.top_row, &lhs.top_row + 4,
                                       &rhs.top_row, &rhs.top_row + 4 );
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1108_window_corners_pack const& rhs )
{
  return os << "{ Upper Left: ( "
            << rhs.left_column << ", " << rhs.top_row << " ), "
            << "Lower Right: ( "
            << rhs.right_column << ", " << rhs.bottom_row << " ) }";
}

// ----------------------------------------------------------------------------
klv_1108_window_corners_pack_format
::klv_1108_window_corners_pack_format() : klv_data_format_< data_type >{ 0 }
{}

// ----------------------------------------------------------------------------
klv_1108_window_corners_pack
klv_1108_window_corners_pack_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const begin = data;
  auto const remaining_length =
    [ & ]() -> size_t { return length - std::distance( begin, data ); };
  klv_1108_window_corners_pack result;
  result.top_row =
    klv_read_ber_oid< uint16_t >( data, remaining_length() );
  result.left_column =
    klv_read_ber_oid< uint16_t >( data, remaining_length() );
  result.bottom_row =
    klv_read_ber_oid< uint16_t >( data, remaining_length() );
  result.right_column =
    klv_read_ber_oid< uint16_t >( data, remaining_length() );
  return result;
}

// ----------------------------------------------------------------------------
void
klv_1108_window_corners_pack_format
::write_typed( klv_1108_window_corners_pack const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const begin = data;
  auto const remaining_length =
    [ & ]() -> size_t { return length - std::distance( begin, data ); };
  klv_write_ber_oid( value.top_row,      data, remaining_length() );
  klv_write_ber_oid( value.left_column,  data, remaining_length() );
  klv_write_ber_oid( value.bottom_row,   data, remaining_length() );
  klv_write_ber_oid( value.right_column, data, remaining_length() );
}

// ----------------------------------------------------------------------------
size_t
klv_1108_window_corners_pack_format
::length_of_typed( klv_1108_window_corners_pack const& value,
                   VITAL_UNUSED size_t length_hint ) const
{
  return klv_ber_oid_length( value.top_row ) +
         klv_ber_oid_length( value.left_column ) +
         klv_ber_oid_length( value.bottom_row ) +
         klv_ber_oid_length( value.right_column );
}

// ----------------------------------------------------------------------------
std::string
klv_1108_window_corners_pack_format
::description() const
{
  return "window corners pack of " + length_description();
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1108_tag tag )
{
  return os << klv_1108_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
klv_1108_local_set_format
::klv_1108_local_set_format()
  : klv_local_set_format{ klv_1108_traits_lookup() }
{}

// ----------------------------------------------------------------------------
klv_local_set
klv_1108_local_set_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  if( length < checksum_packet_length )
  {
    VITAL_THROW( kv::metadata_exception,
                 "checksum not present at end of packet" );
  }

  // Extract checksum key and length
  auto checksum_it = data + ( length - checksum_packet_length );
  auto const checksum_tag = klv_read_int< uint8_t >( checksum_it, 1 );
  auto const checksum_length = klv_read_int< uint8_t >( checksum_it, 1 );
  if( checksum_tag != KLV_1108_CHECKSUM || checksum_length != 2 )
  {
    VITAL_THROW( kv::metadata_exception,
                 "checksum not present at end of packet" );
  }

  // Extract checksum value
  auto const checksum_value_begin = checksum_it;
  auto const checksum_value =
    klv_read_int< uint16_t >( checksum_it, checksum_length );

  // Compare against checksum of rest of KLV packet
  auto const calculated_checksum =
    klv_crc_16_ccitt( data, checksum_value_begin );
  if( calculated_checksum != checksum_value )
  {
    std::stringstream ss;
    ss << std::hex << std::setfill( '0' );
    ss  << "calculated checksum "
        << "(0x" << std::setw( 4 ) << calculated_checksum << ") "
        << "does not equal checksum at end of packet "
        << "(0x" << std::setw( 4 ) << checksum_value << ")";
    VITAL_THROW( kv::metadata_exception, ss.str() );
  }

  // Read rest of packet as normal
  auto const result =
    klv_local_set_format::read_typed( data, length - checksum_packet_length );

  // Ensure iterator ends up after checksum packet
  data += checksum_packet_length;

  return result;
}

// ----------------------------------------------------------------------------
void
klv_1108_local_set_format
::write_typed( klv_local_set const& value,
               klv_write_iter_t& data, size_t length ) const
{
  // Write rest of packet as normal
  auto const begin = data;
  auto const internal_length = length - checksum_packet_length;
  klv_local_set_format::write_typed( value, data, internal_length );

  // Write checksum key and length, which will be included in the checksum
  // calculation
  klv_write_int< uint8_t >( KLV_1108_CHECKSUM, data, 1 );
  klv_write_int< uint8_t >( 2, data, 1 );

  // Write checksum value
  auto const checksum = klv_crc_16_ccitt( begin, data );
  klv_write_int( checksum, data, 2 );
}

// ----------------------------------------------------------------------------
size_t
klv_1108_local_set_format
::length_of_typed( klv_local_set const& value, size_t length_hint ) const
{
  return klv_local_set_format::length_of_typed(
    value, length_hint - checksum_packet_length ) + checksum_packet_length;
}

// ----------------------------------------------------------------------------
std::string
klv_1108_local_set_format
::description() const
{
  return "ST 1108 local set of " + length_description();
}

// ----------------------------------------------------------------------------
klv_uds_key
klv_1108_key()
{
  // From Section 6 of https://gwg.nga.mil/misb/docs/standards/ST1108.3.pdf
  return { 0x060E2B3402030101, 0x0E0103031C000000 };
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_1108_traits_lookup()
{
  // From Table 1 of https://gwg.nga.mil/misb/docs/standards/ST1108.3.pdf
  // Descriptions are edited for clarity, brevity, consistency, etc.
  static klv_tag_traits_lookup const lookup = {
#define ENUM_AND_NAME( X ) X, #X
    { {},
      ENUM_AND_NAME( KLV_1108_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { // "Key" column
      { 0x060E2B3401010101, 0x0E01050100000000 },
      // KWIVER enum
      ENUM_AND_NAME( KLV_1108_ASSESSMENT_POINT ),
      // "Type" column
      std::make_shared< klv_1108_assessment_point_format >(),
      // "Item Name" column
      "Assessment Point",
      // "Notes" column
      "Location in workflow where the metric was evaluated.",
      // "M/O" column
      1 },
    { { 0x060E2B3402050101, 0x0E01050200000000 },
      ENUM_AND_NAME( KLV_1108_METRIC_PERIOD_PACK ),
      std::make_shared< klv_1108_metric_period_pack_format >(),
      "Metric Period Pack",
      "Period for which the metric was evaluated.",
      1 },
    { { 0x060E2B3402050101, 0x0E01030201010000 },
      ENUM_AND_NAME( KLV_1108_WINDOW_CORNERS_PACK ),
      std::make_shared< klv_1108_window_corners_pack_format >(),
      "Window Corners Pack",
      "Image sub-region for which the metric was evaluated.",
      { 0, 1 } },
    { klv_1108_metric_set_key(),
      ENUM_AND_NAME( KLV_1108_METRIC_LOCAL_SET ),
      std::make_shared< klv_1108_metric_local_set_format >(),
      "Metric Local Set",
      "Specification of metrics and their values.",
      { 1, SIZE_MAX } },
    { { 0x060E2B3401010101, 0x0E01050200000000 },
      ENUM_AND_NAME( KLV_1108_COMPRESSION_TYPE ),
      std::make_shared< klv_1108_compression_type_format >(),
      "Compression Type",
      "Type of video compression.",
      1 },
    { { 0x060E2B3401010101, 0x0E01050300000000 },
      ENUM_AND_NAME( KLV_1108_COMPRESSION_PROFILE ),
      std::make_shared< klv_1108_compression_profile_format >(),
      "Compression Profile",
      "Video compression profile.",
      1 },
    { { 0x060E2B3401010101, 0x0E01050400000000 },
      ENUM_AND_NAME( KLV_1108_COMPRESSION_LEVEL ),
      std::make_shared< klv_string_format >(),
      "Compression Level",
      "Level of video compression.",
      1 },
    { { 0x060E2B3401010101, 0x0E01050500000000 },
      ENUM_AND_NAME( KLV_1108_COMPRESSION_RATIO ),
      std::make_shared< klv_float_format >(),
      "Compression Ratio",
      "Source-to-compressed size ratio.",
      1 },
    { { 0x060E2B3401010101, 0x0E01050600000000 },
      ENUM_AND_NAME( KLV_1108_STREAM_BITRATE ),
      std::make_shared< klv_uint_format >( 2 ),
      "Stream Bitrate",
      "Expressed in kilobits / second.",
      1 },
    { { 0x060E2B3401010101, 0x0E01020505000000 },
      ENUM_AND_NAME( KLV_1108_DOCUMENT_VERSION ),
      std::make_shared< klv_uint_format >( 1 ),
      "Document Version",
      "Version number of MISB ST1108.",
      1 },
    { { 0x060E2B3401010101, 0x0E0102035E000000 },
      ENUM_AND_NAME( KLV_1108_CHECKSUM ),
      std::make_shared< klv_uint_format >( 2 ),
      "Checksum",
      "CRC-16-CCITT checksum.",
      0 } };
#undef ENUM_AND_NAME
  return lookup;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
