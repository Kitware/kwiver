// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 1002 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_1002_H_
#define KWIVER_ARROWS_KLV_KLV_1002_H_

#include <arrows/klv/klv_1303.h>
#include <arrows/klv/klv_checksum.h>
#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_set.h>
#include <arrows/klv/klv_util.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/optional.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_1002_tag : klv_lds_key
{
  KLV_1002_UNKNOWN                              = 0,
  KLV_1002_PRECISION_TIMESTAMP                  = 1,

  // Note jump in number here
  KLV_1002_DOCUMENT_VERSION                     = 11,
  KLV_1002_RANGE_IMAGE_ENUMERATIONS             = 12,
  KLV_1002_SPRM                                 = 13,
  KLV_1002_SPRM_UNCERTAINTY                     = 14,
  KLV_1002_SPRM_ROW                             = 15,
  KLV_1002_SPRM_COLUMN                          = 16,
  KLV_1002_NUMBER_SECTIONS_X                    = 17,
  KLV_1002_NUMBER_SECTIONS_Y                    = 18,
  KLV_1002_GENERALIZED_TRANSFORMATION_LOCAL_SET = 19,
  KLV_1002_SECTION_DATA_PACK                    = 20,
  KLV_1002_CHECKSUM                             = 21,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1002_tag tag );

// ----------------------------------------------------------------------------
enum klv_1002_compression_method
{
  KLV_1002_COMPRESSION_METHOD_NONE       = 0,
  KLV_1002_COMPRESSION_METHOD_PLANAR_FIT = 1,
  KLV_1002_COMPRESSION_METHOD_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1002_compression_method value );

// ----------------------------------------------------------------------------
enum klv_1002_data_type
{
  KLV_1002_DATA_TYPE_PERSPECTIVE_RANGE_IMAGE = 0,
  KLV_1002_DATA_TYPE_DEPTH_RANGE_IMAGE       = 1,
  KLV_1002_DATA_TYPE_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1002_data_type value );

// ----------------------------------------------------------------------------
enum klv_1002_source
{
  KLV_1002_SOURCE_COMPUTATIONALLY_EXTRACTED = 0,
  KLV_1002_SOURCE_RANGE_SENSOR              = 1,
  KLV_1002_SOURCE_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1002_source value );

// ----------------------------------------------------------------------------
struct KWIVER_ALGO_KLV_EXPORT klv_1002_enumerations
{
  klv_1002_compression_method compression_method;
  klv_1002_data_type data_type;
  klv_1002_source source;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1002_enumerations const& value );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_1002_enumerations )

// ----------------------------------------------------------------------------
/// Interprets data as a MISB ST1003 enumerations field
class KWIVER_ALGO_KLV_EXPORT klv_1002_enumerations_format
  : public klv_data_format_< klv_1002_enumerations >
{
public:
  klv_1002_enumerations_format();

  std::string
  description() const override;

private:
  klv_1002_enumerations
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_1002_enumerations const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_1002_enumerations const& value ) const override;
};

// ----------------------------------------------------------------------------
struct KWIVER_ALGO_KLV_EXPORT klv_1002_section_data_pack
{
  uint32_t section_x;
  uint32_t section_y;
  klv_1303_mdap< double > measurements;
  kwiver::vital::optional< klv_1303_mdap< double > > uncertainty;
  kwiver::vital::optional< klv_lengthy< double > > plane_x_scale;
  kwiver::vital::optional< klv_lengthy< double > > plane_y_scale;
  kwiver::vital::optional< klv_lengthy< double > > plane_constant;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1002_section_data_pack const& value );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_1002_section_data_pack )

// ----------------------------------------------------------------------------
class KWIVER_ALGO_KLV_EXPORT klv_1002_section_data_pack_format
  : public klv_data_format_< klv_1002_section_data_pack >
{
public:
  klv_1002_section_data_pack_format();

  std::string
  description() const override;

private:
  klv_1002_section_data_pack
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_1002_section_data_pack const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_1002_section_data_pack const& value ) const override;
};

// ----------------------------------------------------------------------------
/// Interprets data as a MISB ST1002 local set.
class KWIVER_ALGO_KLV_EXPORT klv_1002_local_set_format
  : public klv_local_set_format
{
public:
  klv_1002_local_set_format();

  std::string
  description() const override;

  klv_checksum_packet_format const*
  checksum_format() const override;

private:
  klv_crc_16_ccitt_packet_format m_checksum_format;
};

// ----------------------------------------------------------------------------
/// Returns the UDS key for a MISB ST1002 local set.
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_1002_key();

// ----------------------------------------------------------------------------
/// Returns a lookup object for the traits of the ST1002 tags.
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_1002_traits_lookup();

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
