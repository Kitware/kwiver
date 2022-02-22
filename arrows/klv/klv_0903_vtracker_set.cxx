// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0903 VTracker local set parser.

#include "klv_0903_vtracker_set.h"

#include <arrows/klv/klv_0903_location_pack.h>
#include <arrows/klv/klv_series.hpp>
#include <arrows/klv/klv_uuid.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_vtracker_set_tag tag )
{
  return os << klv_0903_vtracker_set_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_detection_status value )
{
  static std::string const
  strings[ KLV_0903_DETECTION_STATUS_ENUM_END + 1 ] = {
    "Inactive",
    "Active",
    "Dropped",
    "Stopped",
    "Unknown Detection Status" };

  os << strings[ std::min( value, KLV_0903_DETECTION_STATUS_ENUM_END ) ];
  return os;
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0903_vtracker_set_traits_lookup()
{
#define ENUM_AND_NAME( X ) X, #X
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKER_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKER_TRACK_ID ),
      std::make_shared< klv_uuid_format >(),
      "Track ID",
      "A unique identifier (UUID) for the track.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKER_DETECTION_STATUS ),
      std::make_shared< klv_0903_detection_status_format >(),
      "Detection Status",
      "Current status of VMTI detections for a given entity.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKER_START_TIME ),
      std::make_shared< klv_uint_format >( 8 ),
      "Start Time",
      "Time of the first observation of the entity. Microseconds since "
      "January 1, 1970.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKER_END_TIME ),
      std::make_shared< klv_uint_format >( 8 ),
      "End Time",
      "Time of the most recent observation of the entity. Microseconds since "
      "January 1, 1970.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKER_BOUNDARY_SERIES ),
      std::make_shared< klv_0903_location_series_format >(),
      "Boundary Series",
      "Set of vertices that specify a minimum bounding area or volume. "
      "Encloses full extent of VMTI detections for the entity.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKER_ALGORITHM ),
      std::make_shared< klv_string_format >(),
      "Algorithm",
      "Name or description of the algorith or method used to create or "
      "maintain object movement reports or predictions.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKER_CONFIDENCE_LEVEL ),
      std::make_shared< klv_uint_format >( 1 ),
      "Confidence Level",
      "Estimation of the certainty or correctness of VMTI movement "
      "detections. Larger values indicate greater confidence.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKER_NUM_TRACK_POINTS ),
      std::make_shared< klv_uint_format >(),
      "Number of Track Points",
      "Number of coordinates which describe the history of VMTI detections "
      "described by `Track History Series`.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKER_TRACK_HISTORY_SERIES ),
      std::make_shared< klv_0903_location_series_format >(),
      "Track History Series",
      "Points that represent the locations of VMTI detections.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKER_VELOCITY ),
      std::make_shared< klv_0903_velocity_pack_format >(),
      "Velocity",
      "Velocity of the entity at the time of last observation.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKER_ACCELERATION ),
      std::make_shared< klv_0903_acceleration_pack_format >(),
      "Acceleration",
      "Acceleration of the entity at the time of last observation.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKER_ALGORITHM_ID ),
      std::make_shared< klv_uint_format >(),
      "Algorithm ID",
      "Identifier indicating which algorithm in the Algorithm Series tracked "
      "this target.",
      { 0, 1 } } };

#undef ENUM_AND_NAME
  return lookup;
}

// ----------------------------------------------------------------------------
klv_0903_vtracker_local_set_format
::klv_0903_vtracker_local_set_format()
  : klv_local_set_format{ klv_0903_vtracker_set_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_0903_vtracker_local_set_format
::description() const
{
  return "vtracker local set of " + length_description();
}

} // namespace arrows

} // namespace kwiver

} // namespace kwiver
