// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0903 algorithm local set parser.

#include "klv_0903_algorithm_set.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_algorithm_set_tag tag )
{
  return os << klv_0903_algorithm_set_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0903_algorithm_set_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0903_ALGORITHM_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0903_ALGORITHM_ID ),
      std::make_shared< klv_uint_format >(),
      "ID",
      "Identifier for the algorithm used. The value 0 is reserved for future "
      "use.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0903_ALGORITHM_NAME ),
      std::make_shared< klv_string_format >(),
      "Name",
      "Name of algorithm.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0903_ALGORITHM_VERSION ),
      std::make_shared< klv_string_format >(),
      "Version",
      "Version of algorithm",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0903_ALGORITHM_CLASS ),
      std::make_shared< klv_string_format >(),
      "Class",
      "Type of algorithm. Examples: 'detector', 'classifier'.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_ALGORITHM_NUM_FRAMES ),
      std::make_shared< klv_uint_format >(),
      "Frame Count",
      "Number of frames the algorithm operates over.",
      { 0, 1 } } };

  return lookup;
}

// ----------------------------------------------------------------------------
klv_0903_algorithm_local_set_format
::klv_0903_algorithm_local_set_format()
  : klv_local_set_format{ klv_0903_algorithm_set_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_0903_algorithm_local_set_format
::description_() const
{
  return "ST0903 Algorithm LS";
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
