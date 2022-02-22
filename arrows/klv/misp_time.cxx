// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Define MISP timestamp utility functions.

#include "misp_time.h"

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

// Indicates a functioning clock of unknown absolute-ness
constexpr uint8_t default_status = 0x9F;

} // namespace

// ----------------------------------------------------------------------------
misp_timestamp
::misp_timestamp()
  : timestamp{ 0 }, status{ default_status } {}

// ----------------------------------------------------------------------------
misp_timestamp
::misp_timestamp( uint64_t timestamp )
  : timestamp{ timestamp }, status{ default_status } {}

// ----------------------------------------------------------------------------
misp_timestamp
::misp_timestamp( uint64_t timestamp, uint8_t status )
  : timestamp{ timestamp }, status{ status } {}

// ----------------------------------------------------------------------------
size_t
misp_timestamp_length()
{
  return misp_detail::packet_length;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
