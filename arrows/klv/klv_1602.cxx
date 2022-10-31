// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 1602 parser.

#include "klv_1602.h"

#include <arrows/klv/klv_util.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1602_tag tag )
{
  return os << klv_1602_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
klv_uds_key
klv_1602_key()
{
  return { 0x060E2B34020B0101, 0x0E01030302000000 };
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_1602_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_1602_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { { 0x060E2B3401010103, 0x0702010101050000 },
      ENUM_AND_NAME( KLV_1602_TIMESTAMP ),
      std::make_shared< klv_uint_format >( 8 ),
      "Precision Timestamp",
      "MISP precision timestamp in microseconds since January 1, 1970.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01020505000000 },
      ENUM_AND_NAME( KLV_1602_VERSION ),
      std::make_shared< klv_ber_oid_format >(),
      "Document Version",
      "Version number of the ST1602 document used to encode this metadata.",
      1 },
    { { 0x060E2B3401010101, 0x0E01010340000000 },
      ENUM_AND_NAME( KLV_1602_SOURCE_IMAGE_ROWS ),
      std::make_shared< klv_uint_format >(),
      "Source Image Rows",
      "Height of the source image in pixels.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E010103401000000 },
      ENUM_AND_NAME( KLV_1602_SOURCE_IMAGE_COLUMNS ),
      std::make_shared< klv_uint_format >(),
      "Source Image Columns",
      "Width of the source image in pixels.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01010340A000000 },
      ENUM_AND_NAME( KLV_1602_SOURCE_IMAGE_AOI_ROWS ),
      std::make_shared< klv_uint_format >(),
      "Source Image AOI Rows",
      "Height of the area of interest in pixels.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01010340C000000 },
      ENUM_AND_NAME( KLV_1602_SOURCE_IMAGE_AOI_COLUMNS ),
      std::make_shared< klv_uint_format >(),
      "Source Image AOI Columns",
      "Width of the area of interest in pixels.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01010340D000000 },
      ENUM_AND_NAME( KLV_1602_SOURCE_IMAGE_AOI_POSITION_X ),
      std::make_shared< klv_sint_format >(),
      "Source Image AOI Position X",
      "X position of the area of interest in pixels. The origin is the top "
      "left corner.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01010340E000000 },
      ENUM_AND_NAME( KLV_1602_SOURCE_IMAGE_AOI_POSITION_Y ),
      std::make_shared< klv_sint_format >(),
      "Source Image AOI Position Y",
      "Y position of the area of interest in pixels. The origin is the top "
      "left corner.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E010103402000000 },
      ENUM_AND_NAME( KLV_1602_SUB_IMAGE_ROWS ),
      std::make_shared< klv_uint_format >(),
      "Sub-Image Rows",
      "Height of the sub-image in pixels.",
      1 },
    { { 0x060E2B3401010101, 0x0E010103403000000 },
      ENUM_AND_NAME( KLV_1602_SUB_IMAGE_COLUMNS ),
      std::make_shared< klv_uint_format >(),
      "Sub-Image Columns",
      "Width of the sub-image in pixels.",
      1 },
    { { 0x060E2B3401010101, 0x0E010103404000000 },
      ENUM_AND_NAME( KLV_1602_SUB_IMAGE_POSITION_X ),
      std::make_shared< klv_sint_format >(),
      "Sub-Image Position X",
      "X position of the sub-image in pixels. The origin is the top left "
      "corner.",
      1 },
    { { 0x060E2B3401010101, 0x0E010103405000000 },
      ENUM_AND_NAME( KLV_1602_SUB_IMAGE_POSITION_Y ),
      std::make_shared< klv_sint_format >(),
      "Sub-Image Position Y",
      "Y position of the sub-image in pixels. The origin is the top left "
      "corner.",
      1 },
    { { 0x060E2B3401010101, 0x0E010103406000000 },
      ENUM_AND_NAME( KLV_1602_ACTIVE_SUB_IMAGE_ROWS ),
      std::make_shared< klv_uint_format >(),
      "Active Sub-Image Rows",
      "Height of the active sub-image in pixels.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E010103407000000 },
      ENUM_AND_NAME( KLV_1602_ACTIVE_SUB_IMAGE_COLUMNS ),
      std::make_shared< klv_uint_format >(),
      "Active Sub-Image Columns",
      "Width of the active sub-image in pixels.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E010103408000000 },
      ENUM_AND_NAME( KLV_1602_ACTIVE_SUB_IMAGE_OFFSET_X ),
      std::make_shared< klv_sint_format >(),
      "Active Sub-Image Offset X",
      "X offset of the active sub-image in pixels from the top left corner of "
      "the sub-image.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E010103409000000 },
      ENUM_AND_NAME( KLV_1602_ACTIVE_SUB_IMAGE_OFFSET_Y ),
      std::make_shared< klv_sint_format >(),
      "Active Sub-Image Offset Y",
      "Y offset of the active sub-image in pixels from the top left corner of "
      "the sub-image.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01010340B000000 },
      ENUM_AND_NAME( KLV_1602_TRANSPARENCY ),
      std::make_shared< klv_uint_format >( 1 ),
      "Transparency",
      "Integer value denoting level of image transparency. A value of 0 "
      "denotes full opacity, while a value of 255 denotes full transparency.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01020506000000 },
      ENUM_AND_NAME( KLV_1602_Z_ORDER ),
      std::make_shared< klv_uint_format >( 1 ),
      "Z-Order",
      "Unique integer defining the image's position along the Z-axis. A value "
      "of 0 denotes the bottom-most image.",
      1 } };

  return lookup;
}

// ----------------------------------------------------------------------------
klv_1602_local_set_format
::klv_1602_local_set_format()
  : klv_local_set_format{ klv_1602_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_1602_local_set_format
::description() const
{
  return "composite imaging local set of " + m_length_constraints.description();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
