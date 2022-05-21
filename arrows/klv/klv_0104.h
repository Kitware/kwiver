// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 0104 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_0104_H_
#define KWIVER_ARROWS_KLV_KLV_0104_H_

#include <arrows/klv/klv_set.h>
#include <arrows/klv/kwiver_algo_klv_export.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0104_tag : klv_lds_key
{
  KLV_0104_UNKNOWN,
  KLV_0104_FRAME_CENTER_LATITUDE,
  KLV_0104_FRAME_CENTER_LONGITUDE,
  KLV_0104_FRAME_CENTER_ELEVATION,
  KLV_0104_IMAGE_COORDINATE_SYSTEM,
  KLV_0104_TARGET_WIDTH,
  KLV_0104_START_DATETIME,
  KLV_0104_EVENT_START_DATETIME,
  KLV_0104_USER_DEFINED_TIMESTAMP,
  KLV_0104_CORNER_LATITUDE_POINT_1,
  KLV_0104_CORNER_LATITUDE_POINT_2,
  KLV_0104_CORNER_LATITUDE_POINT_3,
  KLV_0104_CORNER_LATITUDE_POINT_4,
  KLV_0104_CORNER_LONGITUDE_POINT_1,
  KLV_0104_CORNER_LONGITUDE_POINT_2,
  KLV_0104_CORNER_LONGITUDE_POINT_3,
  KLV_0104_CORNER_LONGITUDE_POINT_4,
  KLV_0104_SLANT_RANGE,
  KLV_0104_SENSOR_ROLL_ANGLE,
  KLV_0104_ANGLE_TO_NORTH,
  KLV_0104_OBLIQUITY_ANGLE,
  KLV_0104_PLATFORM_ROLL_ANGLE,
  KLV_0104_PLATFORM_PITCH_ANGLE,
  KLV_0104_PLATFORM_HEADING_ANGLE,
  KLV_0104_HORIZONTAL_FOV,
  KLV_0104_VERTICAL_FOV,
  KLV_0104_DEVICE_ALTITUDE,
  KLV_0104_DEVICE_LATITUDE,
  KLV_0104_DEVICE_LONGITUDE,
  KLV_0104_IMAGE_SOURCE_DEVICE,
  KLV_0104_EPISODE_NUMBER,
  KLV_0104_DEVICE_DESIGNATION,
  KLV_0104_SECURITY_LOCAL_SET,
  KLV_0104_ENUM_END,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0104_tag tag );

// ----------------------------------------------------------------------------
/// Interprets data as an EG0104 universal set.
class KWIVER_ALGO_KLV_EXPORT klv_0104_universal_set_format
  : public klv_universal_set_format
{
public:
  klv_0104_universal_set_format();

  std::string
  description() const override;
};

// ----------------------------------------------------------------------------
/// Returns the UDS key for an EG0104 universal set.
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_0104_key();

// ----------------------------------------------------------------------------
/// Returns a lookup object for the traits of the ST0601 tags.
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0104_traits_lookup();

// ----------------------------------------------------------------------------
/// Converts EG0104 datetime string to a UNIX timestamp.
///
/// \param value UTC datetime string in \c YYYYMMDDThhmmss format.
///
/// \returns Microseconds since Jan. 1, 1970 (UTC).
VITAL_EXPORT
uint64_t
klv_0104_datetime_to_unix_timestamp( std::string const& value );

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
