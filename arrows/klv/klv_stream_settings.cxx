// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of settings structure for the creation of a klv stream.

#include "klv_stream_settings.h"

#include <climits>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_stream_settings
::klv_stream_settings()
  : type{ KLV_STREAM_TYPE_ASYNC },
    index{ INT_MIN }
{}

// ----------------------------------------------------------------------------
bool operator==( klv_stream_settings const& lhs,
                 klv_stream_settings const& rhs )
{
  return lhs.type == rhs.type && lhs.index == rhs.index;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
