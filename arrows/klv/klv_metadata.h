// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// This file contains the interface for the KLV specialization of the
/// vital::metadata class.

#ifndef KWIVER_ARROWS_KLV_KLV_METADATA_H_
#define KWIVER_ARROWS_KLV_KLV_METADATA_H_

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/misp_time.h>

#include <vital/types/metadata.h>

#include <optional>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
class KWIVER_ALGO_KLV_EXPORT klv_metadata : public kwiver::vital::metadata
{
public:
  virtual ~klv_metadata() = default;

  vital::metadata* clone() const;

  std::vector< klv_packet > const& klv() const;
  std::vector< klv_packet >& klv();

  std::optional< misp_timestamp > const& frame_timestamp() const;
  std::optional< misp_timestamp >& frame_timestamp();

private:
  std::vector< klv_packet > m_klv_packets;
  std::optional< misp_timestamp > m_frame_timestamp;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
