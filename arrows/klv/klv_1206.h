// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 1206 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_1206_H_
#define KWIVER_ARROWS_KLV_KLV_1206_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <arrows/klv/klv_set.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_1206_tag : klv_lds_key
{
  KLV_1206_UNKNOWN                                     = 0,
  KLV_1206_GRAZING_ANGLE                               = 1,
  KLV_1206_GROUND_PLANE_SQUINT_ANGLE                   = 2,
  KLV_1206_LOOK_DIRECTION                              = 3,
  KLV_1206_IMAGE_PLANE                                 = 4,
  KLV_1206_RANGE_RESOLUTION                            = 5,
  KLV_1206_CROSS_RANGE_RESOLUTION                      = 6,
  KLV_1206_RANGE_IMAGE_PLANE_PIXEL_SIZE                = 7,
  KLV_1206_CROSS_RANGE_IMAGE_PLANE_PIXEL_SIZE          = 8,
  KLV_1206_IMAGE_ROWS                                  = 9,
  KLV_1206_IMAGE_COLUMNS                               = 10,
  KLV_1206_RANGE_DIRECTION_ANGLE                       = 11,
  KLV_1206_TRUE_NORTH                                  = 12,
  KLV_1206_RANGE_LAYOVER_ANGLE                         = 13,
  KLV_1206_GROUND_APERTURE_ANGULAR_EXTENT              = 14,
  KLV_1206_APERTURE_DURATION                           = 15,
  KLV_1206_GROUND_TRACK_ANGLE                          = 16,
  KLV_1206_MINIMUM_DETECTABLE_VELOCITY                 = 17,
  KLV_1206_TRUE_PULSE_REPETITION_FREQUENCY             = 18,
  KLV_1206_PULSE_REPETITION_FREQUENCY_SCALE_FACTOR     = 19,
  KLV_1206_TRANSMIT_RF_CENTER_FREQUENCY                = 20,
  KLV_1206_TRANSMIT_RF_BANDWIDTH                       = 21,
  KLV_1206_RADAR_CROSS_SECTION_SCALE_FACTOR_POLYNOMIAL = 22,
  KLV_1206_REFERENCE_FRAME_PRECISION_TIMESTAMP         = 23,
  KLV_1206_REFERENCE_FRAME_GRAZING_ANGLE               = 24,
  KLV_1206_REFERENCE_FRAME_GROUND_PLANE_SQUINT_ANGLE   = 25,
  KLV_1206_REFERENCE_FRAME_RANGE_DIRECTION_ANGLE       = 26,
  KLV_1206_REFERENCE_FRAME_RANGE_LAYOVER_ANGLE         = 27,
  KLV_1206_DOCUMENT_VERSION                            = 28,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1206_tag tag );

// ----------------------------------------------------------------------------
enum klv_1206_look_direction
{
  KLV_1206_LOOK_DIRECTION_LEFT  = 0,
  KLV_1206_LOOK_DIRECTION_RIGHT = 1,
  KLV_1206_LOOK_DIRECTION_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1206_look_direction tag );

// ----------------------------------------------------------------------------
/// Interprets data as a ST1206 look direction.
using klv_1206_look_direction_format =
  klv_enum_format< klv_1206_look_direction >;

// ----------------------------------------------------------------------------
enum klv_1206_image_plane
{
  KLV_1206_IMAGE_PLANE_SLANT  = 0,
  KLV_1206_IMAGE_PLANE_GROUND = 1,
  KLV_1206_IMAGE_PLANE_OTHER  = 2,
  KLV_1206_IMAGE_PLANE_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1206_image_plane tag );

// ----------------------------------------------------------------------------
/// Interprets data as a ST1206 image plane.
using klv_1206_image_plane_format =
  klv_enum_format< klv_1206_image_plane >;

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_1206_key();

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_1206_traits_lookup();

// ----------------------------------------------------------------------------
/// Interprets data as a ST1206 SAR Motion Imagery local set.
class KWIVER_ALGO_KLV_EXPORT klv_1206_local_set_format
  :  public klv_local_set_format
{
public:
  klv_1206_local_set_format();

  std::string
  description() const override;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
