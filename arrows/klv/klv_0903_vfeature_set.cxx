// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0903 VFeature local set parser.

#include <arrows/klv/klv_0903_vfeature_set.h>

#include <arrows/klv/klv_string.h>
#include <arrows/klv/klv_util.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_vfeature_set_tag tag )
{
  return os << klv_0903_vfeature_set_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0903_vfeature_set_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0903_VFEATURE_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0903_VFEATURE_SCHEMA ),
      std::make_shared< klv_utf_8_format >(),
      "Schema",
      "URI which points to a relevant Observation schema "
      "(http://schemas.opengis.net/om/1.0.0/) or a related schema.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VFEATURE_SCHEMA_FEATURE ),
      std::make_shared< klv_utf_8_format >(),
      "Schema Feature",
      "Geographic Markup Language document structured according to the Schema "
      "tag. May contain one or more observed values for a feature of "
      "interest.",
      { 0, 1 } }, };
  return lookup;
}

// ----------------------------------------------------------------------------
klv_0903_vfeature_local_set_format
::klv_0903_vfeature_local_set_format()
  : klv_local_set_format{ klv_0903_vfeature_set_traits_lookup() } {}

// ----------------------------------------------------------------------------
std::string
klv_0903_vfeature_local_set_format
::description_() const
{
  return "ST0903 VFeature LS";
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
