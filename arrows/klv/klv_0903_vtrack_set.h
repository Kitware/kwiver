// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 0903 VTrack local set parser.

#ifndef KWIVER_ARROWS_KLV_KLV_0903_VTRACK_SET_H
#define KWIVER_ARROWS_KLV_KLV_0903_VTRACK_SET_H

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_set.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0903_vtrack_set_tag : klv_lds_key
{
  KLV_0903_VTRACK_UNKNOWN          = 0,
  KLV_0903_VTRACK_CHECKSUM         = 1,
  KLV_0903_VTRACK_TIMESTAMP        = 2,
  KLV_0903_VTRACK_ID               = 3,
  KLV_0903_VTRACK_STATUS           = 4,
  KLV_0903_VTRACK_START_TIME       = 5,
  KLV_0903_VTRACK_END_TIME         = 6,
  KLV_0903_VTRACK_BOUNDARY_SERIES  = 7,
  KLV_0903_VTRACK_ALGORITHM        = 8,
  KLV_0903_VTRACK_CONFIDENCE       = 9,
  KLV_0903_VTRACK_SYSTEM_NAME      = 10,
  KLV_0903_VTRACK_VERSION          = 11,
  KLV_0903_VTRACK_SOURCE_SENSOR    = 12,
  KLV_0903_VTRACK_NUM_TRACK_POINTS = 13,

  // Note the jumps in tag number here
  KLV_0903_VTRACK_ITEM_SERIES      = 101,
  KLV_0903_VTRACK_ONTOLOGY_SERIES  = 103,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_vtrack_set_tag tag );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0903_vtrack_set_traits_lookup();

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 VTrack local set.
///
/// \warning The VTrack set is deprecated as of ST0903.6.
class KWIVER_ALGO_KLV_EXPORT klv_0903_vtrack_local_set_format
  : public klv_local_set_format
{
public:
  klv_0903_vtrack_local_set_format();

  std::string
  description_() const override;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
