// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 1303 parser's non-templated functions.

#include "klv_1303.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1303_apa value )
{
  static std::string strings[ KLV_1303_APA_ENUM_END ] = {
    "Unknown APA",
    "Natural",
    "IMAP",
    "Boolean",
    "Uint",
    "RLE" };

  return os << strings[ ( value >= KLV_1303_APA_ENUM_END )
                        ? KLV_1303_APA_UNKNOWN
                        : value ];
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
