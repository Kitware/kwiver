// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 1002 parser.

#include "klv_1002.h"

#include <arrows/klv/klv_1202.h>
#include <arrows/klv/klv_1303.h>
#include <arrows/klv/klv_checksum.h>
#include <arrows/klv/klv_length_value.h>
#include <arrows/klv/klv_util.h>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1002_tag tag )
{
  return os << klv_1002_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1002_compression_method value )
{
  static std::string strings[ KLV_1002_COMPRESSION_METHOD_ENUM_END + 1 ] = {
    "None",
    "Planar Fit",
    "Unknown Compression Method" };

  return
    os << strings[ std::min( value, KLV_1002_COMPRESSION_METHOD_ENUM_END ) ];
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1002_data_type value )
{
  static std::string strings[ KLV_1002_DATA_TYPE_ENUM_END + 1 ] = {
    "Perspective Range Image",
    "Depth Range Image",
    "Unknown Data Type" };

  return
    os << strings[ std::min( value, KLV_1002_DATA_TYPE_ENUM_END ) ];
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1002_source value )
{
  static std::string strings[ KLV_1002_SOURCE_ENUM_END + 1 ] = {
    "Computational Extracted",
    "Range Sensor",
    "Unknown Source" };

  return
    os << strings[ std::min( value, KLV_1002_SOURCE_ENUM_END ) ];
}

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1002_enumerations const& value )
{
  os << "{ "
     << "compression method: " << value.compression_method << ", "
     << "data type: " << value.data_type << ", "
     << "source: " << value.source
     << " }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_1002_enumerations,
  &klv_1002_enumerations::compression_method,
  &klv_1002_enumerations::data_type,
  &klv_1002_enumerations::source )

// ----------------------------------------------------------------------------
klv_1002_enumerations_format
::klv_1002_enumerations_format()
{}

// ----------------------------------------------------------------------------
std::string
klv_1002_enumerations_format
::description() const
{
  return "range image enumerations of " + m_length_constraints.description();
}

// ----------------------------------------------------------------------------
klv_1002_enumerations
klv_1002_enumerations_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const int_value = klv_read_ber_oid< uint8_t >( data, length );
  return {
    static_cast< klv_1002_compression_method >( int_value & 0x7 ),
    static_cast< klv_1002_data_type >( ( int_value >> 3 ) & 0x7 ),
    static_cast< klv_1002_source >( ( int_value >> 6 ) & 0x1 ) };
}

// ----------------------------------------------------------------------------
void
klv_1002_enumerations_format
::write_typed( klv_1002_enumerations const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const int_value =
    static_cast< uint8_t >( ( value.source << 6 ) |
                            ( value.data_type << 3 ) |
                            ( value.compression_method ) );
  klv_write_ber_oid( int_value, data, length );
}

// ----------------------------------------------------------------------------
size_t
klv_1002_enumerations_format
::length_of_typed( VITAL_UNUSED klv_1002_enumerations const& value ) const
{
  return 1;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1002_section_data_pack const& value )
{
  os << "{ "
     << "section number x: " << value.section_x << ", "
     << "section number y: " << value.section_y << ", "
     << "range measurements: " << value.measurements << ", "
     << "uncertainty: " << value.uncertainty << ", "
     << "plane x-scale factor: " << value.plane_x_scale << ", "
     << "plane y-scale factor: " << value.plane_y_scale << ", "
     << "plane constant: " << value.plane_constant
     << " }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_1002_section_data_pack,
  &klv_1002_section_data_pack::section_x,
  &klv_1002_section_data_pack::section_y,
  &klv_1002_section_data_pack::measurements,
  &klv_1002_section_data_pack::uncertainty,
  &klv_1002_section_data_pack::plane_x_scale,
  &klv_1002_section_data_pack::plane_y_scale,
  &klv_1002_section_data_pack::plane_constant )

namespace section_data_pack_detail {

auto const index_format = klv_ber_oid_format{};
auto const mdap_format =
  klv_1303_mdap_format< klv_lengthless_format< klv_float_format > >{ 8 };
auto const plane_format = klv_float_format{};

} // namespace section_data_pack_detail

// ----------------------------------------------------------------------------
klv_1002_section_data_pack_format
::klv_1002_section_data_pack_format()
{}

// ----------------------------------------------------------------------------
std::string
klv_1002_section_data_pack_format
::description() const
{
  return "section data pack of " + m_length_constraints.description();
}

// ----------------------------------------------------------------------------
klv_1002_section_data_pack
klv_1002_section_data_pack_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  using namespace section_data_pack_detail;

  auto const tracker = track_it( data, length );
  klv_1002_section_data_pack result;

  result.section_x =
    static_cast< uint32_t >(
      klv_read_lv( data, tracker.remaining(), index_format ) );
  result.section_y =
    static_cast< uint32_t >(
      klv_read_lv( data, tracker.remaining(), index_format ) );
  result.measurements =
    klv_read_lv( data, tracker.remaining(), mdap_format );
  result.uncertainty =
    klv_read_opt_lv( data, tracker.remaining(), mdap_format );
  result.plane_x_scale =
    klv_read_trunc_lv( data, tracker.remaining(), plane_format );
  result.plane_y_scale =
    klv_read_trunc_lv( data, tracker.remaining(), plane_format );
  result.plane_constant =
    klv_read_trunc_lv( data, tracker.remaining(), plane_format );

  return result;
}

// ----------------------------------------------------------------------------
void
klv_1002_section_data_pack_format
::write_typed( klv_1002_section_data_pack const& value,
               klv_write_iter_t& data, size_t length ) const
{
  using namespace section_data_pack_detail;

  auto const tracker = track_it( data, length );

  klv_write_lv( value.section_x, data, tracker.remaining(), index_format );
  klv_write_lv( value.section_y, data, tracker.remaining(), index_format );
  klv_write_lv( value.measurements, data, tracker.remaining(), mdap_format );
  klv_write_opt_lv(
    value.uncertainty, data, tracker.remaining(), mdap_format );
  klv_write_trunc_lv( std::forward_as_tuple( value.plane_x_scale,
                                             value.plane_y_scale,
                                             value.plane_constant ),
                      data, tracker.remaining(),
                      plane_format, plane_format, plane_format );
}

// ----------------------------------------------------------------------------
size_t
klv_1002_section_data_pack_format
::length_of_typed( klv_1002_section_data_pack const& value ) const
{
  using namespace section_data_pack_detail;
  return
    klv_length_of_lv( value.section_x, index_format ) +
    klv_length_of_lv( value.section_y, index_format ) +
    klv_length_of_lv( value.measurements, mdap_format ) +
    klv_length_of_opt_lv( value.uncertainty, mdap_format ) +
    klv_length_of_trunc_lv( std::forward_as_tuple( value.plane_x_scale,
                                                   value.plane_y_scale,
                                                   value.plane_constant ),
                            plane_format, plane_format, plane_format );
}

// ----------------------------------------------------------------------------
klv_1002_local_set_format
::klv_1002_local_set_format()
  : klv_local_set_format{ klv_1002_traits_lookup() },
    m_checksum_format{ { KLV_1002_CHECKSUM, 2 } }
{}

// ----------------------------------------------------------------------------
std::string
klv_1002_local_set_format
::description() const
{
  return "range image local set of " + m_length_constraints.description();
}

// ----------------------------------------------------------------------------
klv_checksum_packet_format const*
klv_1002_local_set_format
::checksum_format() const
{
  return &m_checksum_format;
}

// ----------------------------------------------------------------------------
/// Returns the UDS key for a MISB ST1002 local set.
klv_uds_key
klv_1002_key()
{
  return { 0x060E2B34020B0101, 0x0E0103030C000000 };
}

// ----------------------------------------------------------------------------
/// Returns a lookup object for the traits of the ST1002 tags.
klv_tag_traits_lookup const&
klv_1002_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_1002_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { { 0x060E2B3401010103, 0x0702010101050000 },
      ENUM_AND_NAME( KLV_1002_PRECISION_TIMESTAMP ),
      std::make_shared< klv_uint_format >( 8 ),
      "Precision Timestamp",
      "MISP Precision Timestamp, in microseconds since January 1, 1970, "
      "for this metadata.",
      1 },
    { { 0x060E2B3401010101, 0x0E01020505000000 },
      ENUM_AND_NAME( KLV_1002_DOCUMENT_VERSION ),
      std::make_shared< klv_ber_oid_format >(),
      "Document Version",
      "Version number of the MISB ST1002 document used to encode this "
      "metadata.",
      1 },
    { { 0x060E2B3401010101, 0x0E01020360000000 },
      ENUM_AND_NAME( KLV_1002_RANGE_IMAGE_ENUMERATIONS ),
      std::make_shared< klv_1002_enumerations_format >(),
      "Range Image Enumerations",
      "Various essential characteristics of the range imagery expressed as "
      "enumerations.",
      1 },
    { { 0x060E2B3401010101, 0x0E0101033E040000 },
      ENUM_AND_NAME( KLV_1002_SPRM ),
      std::make_shared< klv_float_format >(),
      "Range Measurement",
      "Distance from the principle point to a point in the scene. "
      "Measured in meters.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033E050000 },
      ENUM_AND_NAME( KLV_1002_SPRM_UNCERTAINTY ),
      std::make_shared< klv_float_format >(),
      "Range Measurement Uncertainty",
      "Uncertainty of the range measurement. Measured in meters.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01020507000000 },
      ENUM_AND_NAME( KLV_1002_SPRM_ROW ),
      std::make_shared< klv_float_format >(),
      "Range Measurement Row Coordinate",
      "Row coordinate in the image where the measurement was taken.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01020508000000 },
      ENUM_AND_NAME( KLV_1002_SPRM_COLUMN ),
      std::make_shared< klv_float_format >(),
      "Range Measurement Column Coordinate",
      "Column coordinate in the image where the measurement was taken.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033E000000 },
      ENUM_AND_NAME( KLV_1002_NUMBER_SECTIONS_X ),
      std::make_shared< klv_ber_oid_format >(),
      "Number of Sections in X",
      "Number of sections of the image differentiable along the x axis.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101033E010000 },
      ENUM_AND_NAME( KLV_1002_NUMBER_SECTIONS_Y ),
      std::make_shared< klv_ber_oid_format >(),
      "Number of Sections in Y",
      "Number of sections of the image differentiable along the y axis.",
      { 0, 1 } },
    { { 0x060E2B34020B0101, 0x0E01030505000000 },
      ENUM_AND_NAME( KLV_1002_GENERALIZED_TRANSFORMATION_LOCAL_SET ),
      std::make_shared< klv_1202_local_set_format >(),
      "Generalized Transformation Local Set",
      "Mathematical transformation mapping the child range image to the "
      "parent collaborative sensors image.",
      { 0, 1 },
      &klv_1202_traits_lookup() },
    { { 0x060E2B3402040101, 0x0E01030301000000 },
      ENUM_AND_NAME( KLV_1002_SECTION_DATA_PACK ),
      std::make_shared< klv_1002_section_data_pack_format >(),
      "Section Data Pack",
      "Information describing a section of the image.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0102035E000000 },
      ENUM_AND_NAME( KLV_1002_CHECKSUM ),
      std::make_shared< klv_uint_format >( 2 ),
      "Checksum",
      "CRC-16-CCITT checksum.",
      0 } };

  return lookup;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
