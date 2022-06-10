// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Keys for unimplemented KLV structures.

#include "klv_unimplemented.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
klv_uds_key
klv_0602_key()
{
  return { 0x060E2B3401010101, 0x0E01030301000000 };
}

// ----------------------------------------------------------------------------
klv_uds_key
klv_0809_key()
{
  return { 0x060E2B34022B0101, 0x0E0103010E000000 };
}

// ----------------------------------------------------------------------------
klv_uds_key
klv_1507_key()
{
  return { 0x060E2B34020B0101, 0x0E01030201000000 };
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
