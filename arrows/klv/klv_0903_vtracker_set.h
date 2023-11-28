// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Interface to the KLV 0903 VTracker local set parser.

#ifndef KWIVER_ARROWS_KLV_KLV_0903_VTRACKER_SET_H
#define KWIVER_ARROWS_KLV_KLV_0903_VTRACKER_SET_H

#include <arrows/klv/kwiver_algo_klv_export.h>

#include <arrows/klv/klv_packet.h>
#include <arrows/klv/klv_set.h>

#include <ostream>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
enum klv_0903_vtracker_set_tag : klv_lds_key
{
  KLV_0903_VTRACKER_UNKNOWN                 = 0,
  KLV_0903_VTRACKER_TRACK_ID                = 1,
  KLV_0903_VTRACKER_DETECTION_STATUS        = 2, // Deprecated
  KLV_0903_VTRACKER_FIRST_OBSERVATION_TIME  = 3,
  KLV_0903_VTRACKER_LATEST_OBSERVATION_TIME = 4,
  KLV_0903_VTRACKER_TRACK_BOUNDARY_SERIES   = 5,
  KLV_0903_VTRACKER_ALGORITHM               = 6, // Deprecated
  KLV_0903_VTRACKER_CONFIDENCE_LEVEL        = 7,
  KLV_0903_VTRACKER_NUM_TRACK_POINTS        = 8, // Deprecated
  KLV_0903_VTRACKER_TRACK_HISTORY_SERIES    = 9,
  KLV_0903_VTRACKER_VELOCITY                = 10,
  KLV_0903_VTRACKER_ACCELERATION            = 11,
  KLV_0903_VTRACKER_ALGORITHM_ID            = 12,
};

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_vtracker_set_tag tag );

// ----------------------------------------------------------------------------
enum klv_0903_detection_status
{
  KLV_0903_DETECTION_STATUS_INACTIVE        = 0,
  KLV_0903_DETECTION_STATUS_ACTIVE_MOVING   = 1,
  KLV_0903_DETECTION_STATUS_DROPPED         = 2,
  KLV_0903_DETECTION_STATUS_ACTIVE_STOPPED  = 3,
  KLV_0903_DETECTION_STATUS_ACTIVE_COASTING = 4,
  KLV_0903_DETECTION_STATUS_ENUM_END,
};

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 detection status.
using klv_0903_detection_status_format =
  klv_enum_format< klv_0903_detection_status >;

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
std::ostream&
operator<<( std::ostream& os, klv_0903_detection_status value );

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
klv_tag_traits_lookup const&
klv_0903_vtracker_set_traits_lookup();

// ----------------------------------------------------------------------------
/// Interprets data as a ST0903 vTracker local set.
class KWIVER_ALGO_KLV_EXPORT klv_0903_vtracker_local_set_format
  : public klv_local_set_format
{
public:
  klv_0903_vtracker_local_set_format();

  std::string
  description_() const override;
};

} // namespace klv

} // namespace arrows

} // namespace kwiver

#endif
