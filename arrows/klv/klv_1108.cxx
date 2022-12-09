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
  return "metric period pack of " + m_length_constraints.description();
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
::klv_1108_window_corners_pack_format()
{}

// ----------------------------------------------------------------------------
klv_1108_window_corners_pack
klv_1108_window_corners_pack_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );

  auto const y_min = klv_read_ber_oid< uint16_t >( data, tracker.remaining() );
  auto const x_min = klv_read_ber_oid< uint16_t >( data, tracker.remaining() );
  auto const y_max = klv_read_ber_oid< uint16_t >( data, tracker.remaining() );
  auto const x_max = klv_read_ber_oid< uint16_t >( data, tracker.remaining() );
  return { { x_min, y_min, x_max, y_max } };
}

// ----------------------------------------------------------------------------
void
klv_1108_window_corners_pack_format
::write_typed( klv_1108_window_corners_pack const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_write_ber_oid( value.bbox.min_y(), data, tracker.remaining() );
  klv_write_ber_oid( value.bbox.min_x(), data, tracker.remaining() );
  klv_write_ber_oid( value.bbox.max_y(), data, tracker.remaining() );
  klv_write_ber_oid( value.bbox.max_x(), data, tracker.remaining() );
}

// ----------------------------------------------------------------------------
size_t
klv_1108_window_corners_pack_format
::length_of_typed( klv_1108_window_corners_pack const& value ) const
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
  return "window corners pack of " + m_length_constraints.description();
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
  : klv_local_set_format{ klv_1108_traits_lookup() },
    m_checksum_format{ { KLV_1108_CHECKSUM, 2 } }
{}

// ----------------------------------------------------------------------------
klv_checksum_packet_format const*
klv_1108_local_set_format
::checksum_format() const
{
  return &m_checksum_format;
}

// ----------------------------------------------------------------------------
std::string
klv_1108_local_set_format
::description() const
{
  return "ST 1108 local set of " + m_length_constraints.description();
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

  return lookup;
}

// ----------------------------------------------------------------------------
klv_local_set
klv_1108_create_index_set(
  klv_local_set const& parent_set, klv_value const& metric_set_value )
{
  klv_local_set result;
  for( auto const tag : { KLV_1108_ASSESSMENT_POINT,
                          KLV_1108_WINDOW_CORNERS_PACK } )
  {
    auto const it = parent_set.find( tag );
    if( it != parent_set.cend() )
    {
      result.add( tag, it->second );
    }
  }

  if( metric_set_value.valid() )
  {
    auto const& metric_set = metric_set_value.get< klv_local_set >();
    klv_local_set result_metric_set;
    for( auto const tag : { KLV_1108_METRIC_SET_NAME,
                            KLV_1108_METRIC_SET_VERSION,
                            KLV_1108_METRIC_SET_IMPLEMENTER,
                            KLV_1108_METRIC_SET_PARAMETERS } )
    {
      auto const it = metric_set.find( tag );
      if( it != metric_set.cend() )
      {
        result_metric_set.add( tag, it->second );
      }
    }
    result.add( KLV_1108_METRIC_LOCAL_SET, std::move( result_metric_set ) );
  }

  return result;
}

// ----------------------------------------------------------------------------
struct klv_compression_type_pair
{
  std::string vital;
  klv_1108_compression_type klv;
};

// ----------------------------------------------------------------------------
std::vector< klv_compression_type_pair > const&
compression_type_pairs()
{
  static std::vector< klv_compression_type_pair > const pairs = {
    { "N/A", KLV_1108_COMPRESSION_TYPE_UNCOMPRESSED },
    { "H.262", KLV_1108_COMPRESSION_TYPE_H262 },
    { "H.264", KLV_1108_COMPRESSION_TYPE_H264 },
    { "H.265", KLV_1108_COMPRESSION_TYPE_H265 } };

  return pairs;
}

// ----------------------------------------------------------------------------
struct klv_compression_profile_pair
{
  std::string vital;
  klv_1108_compression_profile klv;
};

// ----------------------------------------------------------------------------
std::vector< klv_compression_profile_pair > const&
compression_profile_pairs()
{
  static std::vector< klv_compression_profile_pair > const pairs = {
    { "N/A", KLV_1108_COMPRESSION_PROFILE_UNCOMPRESSED },
    { "Main", KLV_1108_COMPRESSION_PROFILE_MAIN },
    { "Main 10", KLV_1108_COMPRESSION_PROFILE_MAIN_10 },
    { "Constrained Baseline",
      KLV_1108_COMPRESSION_PROFILE_CONSTRAINED_BASELINE },
    { "High", KLV_1108_COMPRESSION_PROFILE_HIGH },
    { "Main 4:2:2 12", KLV_1108_COMPRESSION_PROFILE_MAIN_4_2_2_12 },
    { "Main 4:4:4 12", KLV_1108_COMPRESSION_PROFILE_MAIN_4_4_4_12 },
    { "High 4:2:2", KLV_1108_COMPRESSION_PROFILE_HIGH_4_2_2 },
    { "High 4:4:4 Predictive",
      KLV_1108_COMPRESSION_PROFILE_HIGH_4_4_4_PREDICTIVE },

    // Not technically correct, but these vital values have no ST1108 direct
    // equivalent
    { "Baseline", KLV_1108_COMPRESSION_PROFILE_CONSTRAINED_BASELINE },
    { "Extended", KLV_1108_COMPRESSION_PROFILE_HIGH },
    { "High 10", KLV_1108_COMPRESSION_PROFILE_HIGH },
    { "High 10 Intra", KLV_1108_COMPRESSION_PROFILE_HIGH },
    { "High 4:2:2 Intra", KLV_1108_COMPRESSION_PROFILE_HIGH_4_2_2 },
    { "High 4:4:4", KLV_1108_COMPRESSION_PROFILE_HIGH_4_4_4_PREDICTIVE },
    { "High 4:4:4 Intra", KLV_1108_COMPRESSION_PROFILE_HIGH_4_4_4_PREDICTIVE },
  };

  return pairs;
}

// ----------------------------------------------------------------------------
struct klv_compression_level_pair
{
  std::string vital;
  std::string klv;
};

// ----------------------------------------------------------------------------
std::vector< klv_compression_level_pair > const&
compression_level_pairs_mpeg2()
{
  static std::vector< klv_compression_level_pair > const pairs = {
    { "Low", "LL" },
    { "Main", "ML" },
    { "High-1440", "H14" },
    { "High", "HL" } };

  return pairs;
}

// ----------------------------------------------------------------------------
template< class PairType >
void convert_vital_to_klv_via_pairs(
  kv::metadata const& vital_data, klv_local_set& klv_data,
  kv::vital_metadata_tag vital_tag, klv_1108_tag klv_tag,
  std::vector< PairType > const& pairs )
{
  auto const& item = vital_data.find( vital_tag );
  if( !item || klv_data.has( klv_tag ) )
  {
    return;
  }

  for( auto const& pair : pairs )
  {
    if( pair.vital == item.template get< decltype( PairType::vital ) >() )
    {
      klv_data.add( klv_tag, pair.klv );
    }
  }
}

// ----------------------------------------------------------------------------
bool
klv_1108_fill_in_metadata(
  kv::metadata const& vital_data, klv_local_set& klv_data )
{
  // Assessment point
  if( !klv_data.has( KLV_1108_ASSESSMENT_POINT ) )
  {
    klv_data.add(
      KLV_1108_ASSESSMENT_POINT, KLV_1108_ASSESSMENT_POINT_ARCHIVE );
  }

  // Bitrate
  auto const& bitrate_vital =
    vital_data.find( kv::VITAL_META_VIDEO_BITRATE );
  if( bitrate_vital && !klv_data.has( KLV_1108_STREAM_BITRATE ) )
  {
    // Convert from bps to kbps
    auto const bitrate_klv = ( bitrate_vital.as_uint64() + 500 ) / 1000;
    klv_data.add( KLV_1108_STREAM_BITRATE, bitrate_klv );
  }

  // Compression type
  convert_vital_to_klv_via_pairs(
    vital_data, klv_data, kv::VITAL_META_VIDEO_COMPRESSION_TYPE,
    KLV_1108_COMPRESSION_TYPE, compression_type_pairs() );

  // Compression profile
  convert_vital_to_klv_via_pairs(
    vital_data, klv_data, kv::VITAL_META_VIDEO_COMPRESSION_PROFILE,
    KLV_1108_COMPRESSION_PROFILE, compression_profile_pairs() );

  // Compression level
  if( klv_data.has( KLV_1108_COMPRESSION_TYPE ) )
  {
    if( klv_data.at( KLV_1108_COMPRESSION_TYPE ) ==
        KLV_1108_COMPRESSION_TYPE_H262 )
    {
      convert_vital_to_klv_via_pairs(
          vital_data, klv_data, kv::VITAL_META_VIDEO_COMPRESSION_LEVEL,
          KLV_1108_COMPRESSION_LEVEL, compression_level_pairs_mpeg2() );
    }
    else
    {
      auto const compression_level_vital =
        vital_data.find( kv::VITAL_META_VIDEO_COMPRESSION_LEVEL );
      if( compression_level_vital &&
          !klv_data.has( KLV_1108_COMPRESSION_LEVEL ) )
      {
        klv_data.add(
          KLV_1108_COMPRESSION_LEVEL, compression_level_vital.as_string() );
      }
    }
  }

  // Compression ratio
  auto const& frame_rate_vital =
    vital_data.find( kv::VITAL_META_VIDEO_FRAME_RATE );
  auto const& frame_width_vital =
    vital_data.find( kv::VITAL_META_IMAGE_WIDTH );
  auto const& frame_height_vital =
    vital_data.find( kv::VITAL_META_IMAGE_HEIGHT );
  if( frame_rate_vital && frame_width_vital && frame_height_vital &&
      bitrate_vital && !klv_data.has( KLV_1108_COMPRESSION_RATIO ) )
  {
    auto const compression_ratio_klv =
      24.0 * frame_width_vital.as_uint64() * frame_height_vital.as_uint64() *
      frame_rate_vital.as_double() / bitrate_vital.as_uint64();
    klv_data.add(
      KLV_1108_COMPRESSION_RATIO,
      klv::klv_lengthy< double >{ compression_ratio_klv, 4 } );
  }

  // Standard version
  if( !klv_data.has( KLV_1108_DOCUMENT_VERSION ) )
  {
    klv_data.add( KLV_1108_DOCUMENT_VERSION, uint64_t{ 3 } );
  }

  // Determine if we have values for all tags we are concerned with here
  for( auto const tag : {
    KLV_1108_ASSESSMENT_POINT,
    KLV_1108_COMPRESSION_TYPE,
    KLV_1108_COMPRESSION_PROFILE,
    KLV_1108_COMPRESSION_LEVEL,
    KLV_1108_COMPRESSION_RATIO,
    KLV_1108_STREAM_BITRATE,
    KLV_1108_DOCUMENT_VERSION,
  } )
  {
    if( !klv_data.has( tag ) )
    {
      return false;
    }
  }

  return true;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
