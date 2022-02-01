// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of the KLV 0806 POI Set parser.

#include "klv_0806_poi_set.h"

#include "klv_0806.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0806_poi_set_traits_lookup()
{
#define ENUM_AND_NAME( X ) X, #X
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0806_POI_SET_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown Tag",
      "Unknown tag.",
      0 },
    { { 0x060E2B3401010101, 0x0E01010316000000 },
      ENUM_AND_NAME( KLV_0806_POI_SET_NUMBER ),
      std::make_shared< klv_uint_format >( 2 ),
      "POI Number",
      "Point of interest number.",
      1 },
    { { 0x060E2B3401010101, 0x0E01010317000000 },
      ENUM_AND_NAME( KLV_0806_POI_SET_LATITUDE ),
      std::make_shared< klv_sflint_format >( -90.0, 90.0, 4 ),
      "POI Latitude",
      "Measured in degrees, relative to WGS84 ellipsoid.",
      1 },
    { { 0x060E2B3401010101, 0x0E01010318000000 },
      ENUM_AND_NAME( KLV_0806_POI_SET_LONGITUDE ),
      std::make_shared< klv_sflint_format >( -180.0, 180.0, 4 ),
      "POI Longitude",
      "Measured in degrees, relative to WGS84 ellipsoid.",
      1 },
    { { 0x060E2B3401010101, 0x0E01010319000000 },
      ENUM_AND_NAME( KLV_0806_POI_SET_ALTITUDE ),
      std::make_shared< klv_uflint_format >( -900.0, 19000.0, 2 ),
      "POI Altitude",
      "Measured relative to mean sea level.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101031A000000 },
      ENUM_AND_NAME( KLV_0806_POI_SET_TYPE ),
      std::make_shared< klv_0806_poi_type_format >(),
      "POI Type",
      "Type of this point of interest.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101031B000000 },
      ENUM_AND_NAME( KLV_0806_POI_SET_TEXT ),
      std::make_shared< klv_string_format >(),
      "POI Text",
      "User-defined string.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101031C000000 },
      ENUM_AND_NAME( KLV_0806_POI_SET_SOURCE_ICON ),
      std::make_shared< klv_string_format >(),
      "POI Source Icon",
      "Per MIL-STD-2525B. Icon used in FalconView.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101031D000000 },
      ENUM_AND_NAME( KLV_0806_POI_SET_SOURCE_ID ),
      std::make_shared< klv_string_format >(),
      "POI Source ID",
      "User-defined string.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E0101031E000000 },
      ENUM_AND_NAME( KLV_0806_POI_SET_LABEL ),
      std::make_shared< klv_string_format >(),
      "POI Label",
      "User-defined string.",
      { 0, 1 } },
    { { 0x060E2B3401010101, 0x0E01040301000000 },
      ENUM_AND_NAME( KLV_0806_POI_SET_OPERATION_ID ),
      std::make_shared< klv_string_format >(),
      "Operation ID",
      "Identifier for the duration of the supporting mission or event "
      "associated with the point of interest. Distinct from the platform "
      "mission designation.",
      { 0, 1 } }, };

#undef ENUM_AND_NAME
  return lookup;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0806_poi_set_tag tag )
{
  return os << klv_0806_poi_set_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
klv_0806_poi_set_format
::klv_0806_poi_set_format()
  : klv_local_set_format{ klv_0806_poi_set_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_0806_poi_set_format
::description() const
{
  return "point-of-interest local set of " + length_description();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
