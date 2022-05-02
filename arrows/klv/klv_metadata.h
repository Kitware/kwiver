// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief This file contains the interface for the KLV specialization of the
/// vital::metadata class.

#ifndef KWIVER_ARROWS_KLV_KLV_METADATA_H_
#define KWIVER_ARROWS_KLV_KLV_METADATA_H_

#include <arrows/klv/klv_packet.h>

#include <vital/types/metadata.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
class KWIVER_ALGO_KLV_EXPORT klv_metadata : public kwiver::vital::metadata
{
public:
  virtual ~klv_metadata() = default;

  vital::metadata* clone() const;

  void set_klv( std::vector< klv_packet > const& packets );

  std::vector< klv_packet > const& klv() const;

private:
  std::vector< klv_packet > m_klv_packets;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
