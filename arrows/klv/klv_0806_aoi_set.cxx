// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0806 AOI Set parser.

#include "klv_0806_aoi_set.h"

#include "klv_0806.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0806_aoi_set_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0806_AOI_SET_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown Tag",
      "Unknown tag.",
      0 },
    { { 0x060E2B3401010101, 0x0E01010316000000 },
      ENUM_AND_NAME( KLV_0806_AOI_SET_NUMBER ),
      std::make_shared< klv_uint_format >( 2 ),
      "AOI Number",
      "Area of interest number.",
      1 },
    { { 0x060E2B3401010101, 0x0701020103070100 },
      ENUM_AND_NAME( KLV_0806_AOI_SET_CORNER_LATITUDE_POINT_1 ),
      std::make_shared< klv_sflint_format >( -90.0, 90.0, 4 ),
      "AOI Corner 1 Latitude",
      "Northwest corner of area of interest. Measured in degrees, relative to "
      "WGS84 ellipsoid.",
      1 },
    { { 0x060E2B3401010101, 0x07010201030B0100 },
      ENUM_AND_NAME( KLV_0806_AOI_SET_CORNER_LONGITUDE_POINT_1 ),
      std::make_shared< klv_sflint_format >( -180.0, 180.0, 4 ),
      "AOI Corner 1 Longitude",
      "Northwest corner of area of interest. Measured in degrees, relative to "
      "WGS84 ellipsoid.",
      1 },
    { { 0x060E2B3401010101, 0x0701020103090100 },
      ENUM_AND_NAME( KLV_0806_AOI_SET_CORNER_LATITUDE_POINT_3 ),
      std::make_shared< klv_sflint_format >( -90.0, 90.0, 4 ),
      "AOI Corner 3 Latitude",
      "Southeast corner of area of interest. Measured in degrees, relative to "
      "WGS84 ellipsoid.",
      1 },
    { { 0x060E2B3401010101, 0x07010201030D0100 },
      ENUM_AND_NAME( KLV_0806_AOI_SET_CORNER_LONGITUDE_POINT_3 ),
      std::make_shared< klv_sflint_format >( -180.0, 180.0, 4 ),
      "AOI Corner 3 Longitude",
      "Southeast corner of area of interest. Measured in degrees, relative to "
      "WGS84 ellipsoid.",
      1 },
    { { 0x060E2B3401010101, 0x0E0101031A000000 },
      ENUM_AND_NAME( KLV_0806_AOI_SET_TYPE ),
      std::make_shared< klv_0806_aoi_type_format >(),
      "AOI Type",
      "Type of this point of interest.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101031B000000 },
      ENUM_AND_NAME( KLV_0806_AOI_SET_TEXT ),
      std::make_shared< klv_string_format >(),
      "AOI Text",
      "User-defined string.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101031D000000 },
      ENUM_AND_NAME( KLV_0806_AOI_SET_SOURCE_ID ),
      std::make_shared< klv_string_format >(),
      "AOI Source ID",
      "User-defined string.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101031E000000 },
      ENUM_AND_NAME( KLV_0806_AOI_SET_LABEL ),
      std::make_shared< klv_string_format >(),
      "AOI Label",
      "User-defined string.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01040301000000 },
      ENUM_AND_NAME( KLV_0806_AOI_SET_OPERATION_ID ),
      std::make_shared< klv_string_format >(),
      "Operation ID",
      "Identifier for the duration of the supporting mission or event "
      "associated with the point of interest. Distinct from the platform "
      "mission designation.",
      { 0, 1 } }, };

  return lookup;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0806_aoi_set_tag tag )
{
  return os << klv_0806_aoi_set_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
klv_0806_aoi_set_format
::klv_0806_aoi_set_format()
  : klv_local_set_format{ klv_0806_aoi_set_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_0806_aoi_set_format
::description() const
{
  return "area-of-interest local set of " + length_description();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
