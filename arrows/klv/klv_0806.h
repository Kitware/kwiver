// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 0806 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_0806_H_
#define KWIVER_ARROWS_KLV_KLV_0806_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

#include "klv_checksum.h"
#include "klv_packet.h"
#include "klv_set.h"

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0806_tag : klv_lds_key
{
  KLV_0806_UNKNOWN                                     = 0,
  KLV_0806_CHECKSUM                                    = 1,
  KLV_0806_TIMESTAMP                                   = 2,
  KLV_0806_PLATFORM_TRUE_AIRSPEED                      = 3,
  KLV_0806_PLATFORM_INDICATED_AIRSPEED                 = 4,
  KLV_0806_TELEMETRY_ACCURACY_INDICATOR                = 5,
  KLV_0806_FRAG_CIRCLE_RADIUS                          = 6,
  KLV_0806_FRAME_CODE                                  = 7,
  KLV_0806_VERSION_NUMBER                              = 8,
  KLV_0806_VIDEO_DATA_RATE                             = 9,
  KLV_0806_DIGITAL_VIDEO_FILE_FORMAT                   = 10,
  KLV_0806_USER_DEFINED_LOCAL_SET                      = 11,
  KLV_0806_POI_LOCAL_SET                               = 12,
  KLV_0806_AOI_LOCAL_SET                               = 13,
  KLV_0806_MGRS_ZONE                                   = 14,
  KLV_0806_MGRS_LATITUDE_BAND_GRID_SQUARE              = 15,
  KLV_0806_MGRS_EASTING                                = 16,
  KLV_0806_MGRS_NORTHING                               = 17,
  KLV_0806_FRAME_CENTER_MGRS_ZONE                      = 18,
  KLV_0806_FRAME_CENTER_MGRS_LATITUDE_BAND_GRID_SQUARE = 19,
  KLV_0806_FRAME_CENTER_MGRS_EASTING                   = 20,
  KLV_0806_FRAME_CENTER_MGRS_NORTHING                  = 21,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0806_tag tag );

// ----------------------------------------------------------------------------
/// Interprets data as a MISB ST0806 local set.
class KWIVER_ALGO_KLV_EXPORT klv_0806_local_set_format
  : public klv_local_set_format
{
public:
  klv_0806_local_set_format();

  std::string
  description() const override;

  klv_checksum_packet_format const*
  checksum_format() const override;

private:
  klv_crc_32_mpeg_packet_format m_checksum_format;
};

// ----------------------------------------------------------------------------
/// Returns the UDS key for a MISB ST0806 local set.
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_0806_key();

// ----------------------------------------------------------------------------
/// Returns a lookup object for the traits of the ST0806 tags.
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0806_traits_lookup();

// ----------------------------------------------------------------------------
/// Indicates the type of the point- or area-of-interest.
enum klv_0806_poi_type
{
  KLV_0806_POI_AOI_TYPE_FRIENDLY = 1,
  KLV_0806_POI_AOI_TYPE_HOSTILE  = 2,
  KLV_0806_POI_AOI_TYPE_TARGET   = 3,
  KLV_0806_POI_AOI_TYPE_UNKNOWN  = 4,
  KLV_0806_POI_AOI_TYPE_ENUM_END,
};

using klv_0806_aoi_type = klv_0806_poi_type;

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0806_poi_type tag );

// ----------------------------------------------------------------------------
using klv_0806_poi_type_format = klv_enum_format< klv_0806_poi_type >;
using klv_0806_aoi_type_format = klv_enum_format< klv_0806_aoi_type >;

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
