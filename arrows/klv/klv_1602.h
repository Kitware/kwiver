// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 1602 parser.

#ifndef KWIVER_ARROWS_KLV_KLV_1602_H_
#define KWIVER_ARROWS_KLV_KLV_1602_H_

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_set.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_1602_tag : klv_lds_key
{
  KLV_1602_UNKNOWN                     = 0,
  KLV_1602_TIMESTAMP                   = 1,
  KLV_1602_VERSION                     = 2,
  KLV_1602_SOURCE_IMAGE_ROWS           = 3,
  KLV_1602_SOURCE_IMAGE_COLUMNS        = 4,
  KLV_1602_SOURCE_IMAGE_AOI_ROWS       = 5,
  KLV_1602_SOURCE_IMAGE_AOI_COLUMNS    = 6,
  KLV_1602_SOURCE_IMAGE_AOI_POSITION_X = 7,
  KLV_1602_SOURCE_IMAGE_AOI_POSITION_Y = 8,
  KLV_1602_SUB_IMAGE_ROWS              = 9,
  KLV_1602_SUB_IMAGE_COLUMNS           = 10,
  KLV_1602_SUB_IMAGE_POSITION_X        = 11,
  KLV_1602_SUB_IMAGE_POSITION_Y        = 12,
  KLV_1602_ACTIVE_SUB_IMAGE_ROWS       = 13,
  KLV_1602_ACTIVE_SUB_IMAGE_COLUMNS    = 14,
  KLV_1602_ACTIVE_SUB_IMAGE_OFFSET_X   = 15,
  KLV_1602_ACTIVE_SUB_IMAGE_OFFSET_Y   = 16,
  KLV_1602_TRANSPARENCY                = 17,
  KLV_1602_Z_ORDER                     = 18,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_1602_tag tag );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_1602_key();

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_1602_traits_lookup();

// ----------------------------------------------------------------------------
/// Interprets data as a ST1602 composite imaging local set.
class KWIVER_ALGO_KLV_EXPORT klv_1602_local_set_format
  : public klv_local_set_format
{
public:
  klv_1602_local_set_format();

  std::string
  description() const override;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
