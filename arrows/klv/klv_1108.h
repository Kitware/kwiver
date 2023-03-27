// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 1108 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_1108_H_
#define KWIVER_ARROWS_KLV_KLV_1108_H_

#include <arrows/klv/klv_checksum.h>
#include <arrows/klv/klv_set.h>
#include <arrows/klv/klv_util.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <vital/types/bounding_box.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_1108_tag : klv_lds_key
{
  KLV_1108_UNKNOWN             = 0,
  KLV_1108_ASSESSMENT_POINT    = 1,
  KLV_1108_METRIC_PERIOD_PACK  = 2,
  KLV_1108_WINDOW_CORNERS_PACK = 3,
  KLV_1108_METRIC_LOCAL_SET    = 4,
  KLV_1108_COMPRESSION_TYPE    = 5,
  KLV_1108_COMPRESSION_PROFILE = 6,
  KLV_1108_COMPRESSION_LEVEL   = 7,
  KLV_1108_COMPRESSION_RATIO   = 8,
  KLV_1108_STREAM_BITRATE      = 9,
  KLV_1108_DOCUMENT_VERSION    = 10,
  KLV_1108_CHECKSUM            = 11,
  KLV_1108_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1108_tag tag );

// ----------------------------------------------------------------------------
/// Indicates at what point in the pipeline the metrics were evaluated.
enum KWIVER_ALGO_KLV_EXPORT klv_1108_assessment_point
{
  KLV_1108_ASSESSMENT_POINT_UNKNOWN,
  KLV_1108_ASSESSMENT_POINT_SENSOR,
  KLV_1108_ASSESSMENT_POINT_SENSOR_ENCODER,
  KLV_1108_ASSESSMENT_POINT_GCS_RECEIVED,
  KLV_1108_ASSESSMENT_POINT_GCS_TRANSMIT,
  KLV_1108_ASSESSMENT_POINT_ARCHIVE,
  KLV_1108_ASSESSMENT_POINT_ENUM_END,
};

// ----------------------------------------------------------------------------
/// Interprets data as a KLV 1108 assessment point.
using klv_1108_assessment_point_format =
  klv_enum_format< klv_1108_assessment_point >;

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1108_assessment_point value );

// ----------------------------------------------------------------------------
/// Indicates the standard used to compress the Motion Imagery.
enum KWIVER_ALGO_KLV_EXPORT klv_1108_compression_type
{
  KLV_1108_COMPRESSION_TYPE_UNCOMPRESSED,
  KLV_1108_COMPRESSION_TYPE_H262,
  KLV_1108_COMPRESSION_TYPE_H264,
  KLV_1108_COMPRESSION_TYPE_H265,
  KLV_1108_COMPRESSION_TYPE_JPEG2000,
  KLV_1108_COMPRESSION_TYPE_ENUM_END,
};

// ----------------------------------------------------------------------------
/// Interprets data as a KLV 1108 compression type.
using klv_1108_compression_type_format =
  klv_enum_format< klv_1108_compression_type >;

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1108_compression_type value );

// ----------------------------------------------------------------------------
/// Indicates the profile used to compress the Motion Imagery.
enum KWIVER_ALGO_KLV_EXPORT klv_1108_compression_profile
{
  KLV_1108_COMPRESSION_PROFILE_UNCOMPRESSED,
  KLV_1108_COMPRESSION_PROFILE_MAIN,
  KLV_1108_COMPRESSION_PROFILE_MAIN_10,
  KLV_1108_COMPRESSION_PROFILE_CONSTRAINED_BASELINE,
  KLV_1108_COMPRESSION_PROFILE_HIGH,
  KLV_1108_COMPRESSION_PROFILE_MAIN_4_2_2_12,
  KLV_1108_COMPRESSION_PROFILE_MAIN_4_4_4_12,
  KLV_1108_COMPRESSION_PROFILE_HIGH_4_2_2,
  KLV_1108_COMPRESSION_PROFILE_HIGH_4_4_4_PREDICTIVE,
  KLV_1108_COMPRESSION_PROFILE_ENUM_END,
};

// ----------------------------------------------------------------------------
/// Interprets data as a KLV 1108 compression profile.
using klv_1108_compression_profile_format =
  klv_enum_format< klv_1108_compression_profile >;

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1108_compression_profile value );

// ----------------------------------------------------------------------------
/// Indicates the range of time for which these metrics are valid.
struct KWIVER_ALGO_KLV_EXPORT klv_1108_metric_period_pack
{
  uint64_t timestamp;
  uint32_t offset;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1108_metric_period_pack const& rhs );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_1108_metric_period_pack )

// ----------------------------------------------------------------------------
/// Interprets data as a KLV 1108 metric period pack.
class KWIVER_ALGO_KLV_EXPORT klv_1108_metric_period_pack_format
  : public klv_data_format_< klv_1108_metric_period_pack >
{
public:
  klv_1108_metric_period_pack_format();

  std::string
  description_() const override;

  klv_1108_metric_period_pack
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_1108_metric_period_pack const& value,
               klv_write_iter_t& data, size_t length ) const override;
};

// ----------------------------------------------------------------------------
/// Indicates the bounding box for which the metrics were calculated.
struct KWIVER_ALGO_KLV_EXPORT klv_1108_window_corners_pack
{
  kwiver::vital::bounding_box< uint16_t > bbox;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1108_window_corners_pack const& rhs );

// ----------------------------------------------------------------------------
DECLARE_CMP( klv_1108_window_corners_pack )

// ----------------------------------------------------------------------------
/// Interprets data as a KLV 1108 window corners pack.
class KWIVER_ALGO_KLV_EXPORT klv_1108_window_corners_pack_format
  : public klv_data_format_< klv_1108_window_corners_pack >
{
public:
  klv_1108_window_corners_pack_format();

  std::string
  description_() const override;

private:
  klv_1108_window_corners_pack
  read_typed( klv_read_iter_t& data, size_t length ) const override;

  void
  write_typed( klv_1108_window_corners_pack const& value,
               klv_write_iter_t& data, size_t length ) const override;

  size_t
  length_of_typed( klv_1108_window_corners_pack const& value ) const override;
};

// ----------------------------------------------------------------------------
class KWIVER_ALGO_KLV_EXPORT klv_1108_local_set_format
  : public klv_local_set_format
{
public:
  klv_1108_local_set_format();

  std::string
  description_() const override;

  klv_checksum_packet_format const*
  checksum_format() const override;

private:
  klv_crc_16_ccitt_packet_format m_checksum_format;
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_1108_key();

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_1108_traits_lookup();

// ----------------------------------------------------------------------------
/// Creates a local set which can serve as a ST1108 index.
///
/// Two parent/metric pairs with the same index and different metric values
/// are in contradiction. Two pairs with different indices can coherently have
/// different metric values.
KWIVER_ALGO_KLV_EXPORT
klv_local_set
klv_1108_create_index_set(
  klv_local_set const& parent_set, klv_value const& metric_set_value );

// ----------------------------------------------------------------------------
/// Fills in any ST1108 metadata fields derivable from \p vital_data.
///
/// Any existing values in \p klv_data will not be overwritten.
///
/// \return \c true if all possible klv fields have been filled in.
KWIVER_ALGO_KLV_EXPORT
bool
klv_1108_fill_in_metadata(
  vital::metadata const& vital_data, klv_local_set& klv_data );

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
