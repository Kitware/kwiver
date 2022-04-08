// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief This file contains the implementation for the KLV specialization of
/// the vital::metadata class.

#include "klv_metadata.h"

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
void
klv_metadata
::set_klv( std::vector< klv_packet > const& packets )
{
  m_klv_packets = packets;
}

// ----------------------------------------------------------------------------
std::vector< klv_packet > const&
klv_metadata
::klv() const
{
  return m_klv_packets;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
