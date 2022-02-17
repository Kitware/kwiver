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

constexpr size_t checksum_packet_length = 4;
std::vector< uint8_t > const checksum_header = { KLV_1108_CHECKSUM, 2 };

} // namespace <anonymous>

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
std::ostream&
operator<<( std::ostream& os, klv_1108_metric_period_pack const& rhs )
{
  return os << "{ Timestamp: " << rhs.timestamp << ", "
            << "Offset: " << rhs.offset << " }";
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_1108_metric_period_pack,
  &klv_1108_metric_period_pack::timestamp,
  &klv_1108_metric_period_pack::offset
)

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
namespace {

std::tuple< uint16_t, uint16_t, uint16_t, uint16_t >
tuplize( klv_1108_window_corners_pack const& value )
{
  return std::make_tuple( value.bbox.min_x(), value.bbox.min_y(),
                          value.bbox.max_x(), value.bbox.max_y() );
}

} // namespace

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP_TUPLIZE( klv_1108_window_corners_pack )

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1108_window_corners_pack const& rhs )
{
  return os << "{ Upper Left: ( "
            << rhs.bbox.min_x() << ", " << rhs.bbox.min_y() << " ), "
            << "Lower Right: ( "
            << rhs.bbox.max_x() << ", " << rhs.bbox.max_y() << " ) }";
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

  auto const y_min = klv_read_ber_oid< uint16_t >( data, remaining_length() );
  auto const x_min = klv_read_ber_oid< uint16_t >( data, remaining_length() );
  auto const y_max = klv_read_ber_oid< uint16_t >( data, remaining_length() );
  auto const x_max = klv_read_ber_oid< uint16_t >( data, remaining_length() );
  return { { x_min, y_min, x_max, y_max } };
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
  klv_write_ber_oid( value.bbox.min_y(), data, remaining_length() );
  klv_write_ber_oid( value.bbox.min_x(), data, remaining_length() );
  klv_write_ber_oid( value.bbox.max_y(), data, remaining_length() );
  klv_write_ber_oid( value.bbox.max_x(), data, remaining_length() );
}

// ----------------------------------------------------------------------------
size_t
klv_1108_window_corners_pack_format
::length_of_typed( klv_1108_window_corners_pack const& value,
                   VITAL_UNUSED size_t length_hint ) const
{
  return klv_ber_oid_length( value.bbox.min_y() ) +
         klv_ber_oid_length( value.bbox.min_x() ) +
         klv_ber_oid_length( value.bbox.max_y() ) +
         klv_ber_oid_length( value.bbox.max_x() );
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
uint32_t
klv_1108_local_set_format
::calculate_checksum( klv_read_iter_t data, size_t length ) const
{
  return klv_crc_16_ccitt( checksum_header.begin(), checksum_header.end(),
                           klv_crc_16_ccitt( data, data + length ) );
}

// ----------------------------------------------------------------------------
uint32_t
klv_1108_local_set_format
::read_checksum( klv_read_iter_t data, size_t length ) const
{
  if( length < checksum_packet_length )
  {
    VITAL_THROW( kv::metadata_buffer_overflow,
                 "packet too small; checksum is not present" );
  }
  data += length - checksum_packet_length;

  if( !std::equal( checksum_header.cbegin(), checksum_header.cend(), data ) )
  {
    VITAL_THROW( kv::metadata_exception,
                 "checksum header not present" );
  }
  data += checksum_header.size();

  return klv_read_int< uint16_t >( data, 2 );
}

// ----------------------------------------------------------------------------
void
klv_1108_local_set_format
::write_checksum( uint32_t checksum,
                  klv_write_iter_t& data, size_t max_length ) const
{
  if( max_length < checksum_packet_length )
  {
    VITAL_THROW( kv::metadata_buffer_overflow,
                 "writing checksum packet overflows data buffer" );
  }
  data = std::copy( checksum_header.cbegin(), checksum_header.cend(), data );
  klv_write_int( static_cast< uint16_t >( checksum ), data, 2 );
}

// ----------------------------------------------------------------------------
size_t
klv_1108_local_set_format
::checksum_length() const
{
  return checksum_packet_length;
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
      { 1, SIZE_MAX },
      &klv_1108_metric_set_traits_lookup() },
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
