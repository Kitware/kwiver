// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0903 VChip local set parser.

#include "klv_0903_vchip_set.h"

#include <arrows/klv/klv_util.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_vchip_set_tag tag )
{
  return os << klv_0903_vchip_set_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0903_vchip_set_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0903_VCHIP_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0903_VCHIP_IMAGE_TYPE ),
      std::make_shared< klv_string_format >(),
      "Image Type",
      "IANA image media subtype. Only 'jpeg' and 'png' are permitted.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VCHIP_IMAGE_URI ),
      std::make_shared< klv_string_format >(),
      "Image URI",
      "URI referring to an image stored on a server.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VCHIP_EMBEDDED_IMAGE ),
      std::make_shared< klv_blob_format >(),
      "Embedded Image",
      "Embedded binary image data.",
      { 0, 1 } }, };
  return lookup;
}

// ----------------------------------------------------------------------------
klv_0903_vchip_local_set_format
::klv_0903_vchip_local_set_format()
  : klv_local_set_format{ klv_0903_vchip_set_traits_lookup() } {}

// ----------------------------------------------------------------------------
std::string
klv_0903_vchip_local_set_format
::description_() const
{
  return "ST0903 VChip LS";
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
