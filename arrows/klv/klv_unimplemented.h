// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Key declarations for unimplemented KLV structures.

#include "klv_key.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
/// Return the UDS key for a MISB ST0602 universal set.
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_0602_key();

// ----------------------------------------------------------------------------
/// Return the UDS key for a MISB ST0809 local set.
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_0809_key();

// ----------------------------------------------------------------------------
/// Return the UDS key for a MISB ST1507 local set.
KWIVER_ALGO_KLV_EXPORT
klv_uds_key
klv_1507_key();

} // namespace klv

} // namespace arrows

} // namespace kwiver
