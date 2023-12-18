// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 1107 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_1107_H_
#define KWIVER_ARROWS_KLV_KLV_1107_H_

#include <arrows/klv/klv_checksum.h>
#include <arrows/klv/klv_set.h>
#include <arrows/klv/klv_util.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_1107_tag : klv_lds_key
{
  KLV_1107_UNKNOWN                              = 0,
  KLV_1107_SENSOR_ECEF_POSITION_X               = 1,
  KLV_1107_SENSOR_ECEF_POSITION_Y               = 2,
  KLV_1107_SENSOR_ECEF_POSITION_Z               = 3,
  KLV_1107_SENSOR_ECEF_VELOCITY_X               = 4,
  KLV_1107_SENSOR_ECEF_VELOCITY_Y               = 5,
  KLV_1107_SENSOR_ECEF_VELOCITY_Z               = 6,
  KLV_1107_SENSOR_ABSOLUTE_AZIMUTH              = 7,
  KLV_1107_SENSOR_ABSOLUTE_PITCH                = 8,
  KLV_1107_SENSOR_ABSOLUTE_ROLL                 = 9,
  KLV_1107_SENSOR_ABSOLUTE_AZIMUTH_RATE         = 10,
  KLV_1107_SENSOR_ABSOLUTE_PITCH_RATE           = 11,
  KLV_1107_SENSOR_ABSOLUTE_ROLL_RATE            = 12,
  KLV_1107_BORESIGHT_OFFSET_DELTA_X             = 13,
  KLV_1107_BORESIGHT_OFFSET_DELTA_Y             = 14,
  KLV_1107_BORESIGHT_OFFSET_DELTA_Z             = 15,
  KLV_1107_BORESIGHT_DELTA_ANGLE_1              = 16,
  KLV_1107_BORESIGHT_DELTA_ANGLE_2              = 17,
  KLV_1107_BORESIGHT_DELTA_ANGLE_3              = 18,
  KLV_1107_FOCAL_PLANE_PRINCIPAL_POINT_OFFSET_Y = 19,
  KLV_1107_FOCAL_PLANE_PRINCIPAL_POINT_OFFSET_X = 20,
  KLV_1107_EFFECTIVE_FOCAL_LENGTH               = 21,
  KLV_1107_RADIAL_DISTORTION_CONSTANT           = 22,
  KLV_1107_RADIAL_DISTORTION_PARAMETER_1        = 23,
  KLV_1107_RADIAL_DISTORTION_PARAMETER_2        = 24,
  KLV_1107_RADIAL_DISTORTION_PARAMETER_3        = 25,
  KLV_1107_TANGENTIAL_DISTORTION_PARAMETER_1    = 26,
  KLV_1107_TANGENTIAL_DISTORTION_PARAMETER_2    = 27,
  KLV_1107_TANGENTIAL_DISTORTION_PARAMETER_3    = 28,
  KLV_1107_DIFFERENTIAL_SCALE_AFFINE_PARAMETER  = 29,
  KLV_1107_SKEWNESS_AFFINE_PARAMETER            = 30,
  KLV_1107_SLANT_RANGE                          = 31,
  KLV_1107_SDCC_FLP                             = 32,
  KLV_1107_GENERALIZED_TRANSFORMATION_LOCAL_SET = 33,
  KLV_1107_IMAGE_ROWS                           = 34,
  KLV_1107_IMAGE_COLUMNS                        = 35,
  KLV_1107_PIXEL_SIZE_X                         = 36,
  KLV_1107_PIXEL_SIZE_Y                         = 37,
  KLV_1107_SLANT_RANGE_PEDIGREE                 = 38,
  KLV_1107_LINE_COORDINATE                      = 39,
  KLV_1107_SAMPLE_COORDINATE                    = 40,
  KLV_1107_LRF_DIVERGENCE                       = 41,
  KLV_1107_RADIAL_DISTORTION_VALID_RANGE        = 42,
  KLV_1107_PRECISION_TIMESTAMP                  = 43,
  KLV_1107_DOCUMENT_VERSION                     = 44,
  KLV_1107_CHECKSUM                             = 45,
  KLV_1107_LEAP_SECONDS                         = 46,
  KLV_1107_EFFECTIVE_FOCAL_LENGTH_EXTENDED      = 47,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1107_tag tag );

// ----------------------------------------------------------------------------
enum klv_1107_slant_range_pedigree
{
  KLV_1107_SLANT_RANGE_PEDIGREE_OTHER      = 0,
  KLV_1107_SLANT_RANGE_PEDIGREE_MEASURED   = 1,
  KLV_1107_SLANT_RANGE_PEDIGREE_CALCULATED = 2,
  klv_1107_SLANT_RANGE_PEDIGREE_ENUM_END,
};

// ----------------------------------------------------------------------------
/// Interprets data as a KLV 1107 slant range pedigree.
using klv_1107_slant_range_pedigree_format =
  klv_enum_format< klv_1107_slant_range_pedigree >;

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1107_slant_range_pedigree value );

// ----------------------------------------------------------------------------
class KWIVER_ALGO_KLV_EXPORT klv_1107_local_set_format
  : public klv_local_set_format
{
public:
  klv_1107_local_set_format();

  std::string
  description_() const override;

  klv_checksum_packet_format const*
  packet_checksum_format() const override;

private:
  klv_crc_16_ccitt_packet_format m_checksum_format;
};

// ----------------------------------------------------------------------------
/// Return the UDS key for a MISB ST1107 local set.
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_1107_key();

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_1107_traits_lookup();

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
