// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0903 VTrack local set parser.

#include "klv_0903_vtrack_set.h"

#include <arrows/klv/klv_0903_location_pack.h>
#include <arrows/klv/klv_0903_ontology_set.h>
#include <arrows/klv/klv_0903_vtracker_set.h>
#include <arrows/klv/klv_0903_vtrackitem_pack.h>
#include <arrows/klv/klv_series.hpp>
#include <arrows/klv/klv_uuid.h>

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_vtrack_set_tag tag )
{
  return os << klv_0903_vtrack_set_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0903_vtrack_set_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_CHECKSUM ),
      std::make_shared< klv_uint_format >(),
      "Checksum",
      "Checksum used to detect errors within a ST 0903 packet.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_TIMESTAMP ),
      std::make_shared< klv_uint_format >( 8 ),
      "Precision Timestamp",
      "Microseconds since January 1st, 1970.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_ID ),
      std::make_shared< klv_uuid_format >(),
      "Track ID",
      "A unique identifier (UUID) for the track.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_STATUS ),
      std::make_shared< klv_0903_detection_status_format >(),
      "Detection Status",
      "Current status of VMTI detections for a given entity.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_START_TIME ),
      std::make_shared< klv_uint_format >( 8 ),
      "Start Time",
      "Time of the first observation of the entity. Microseconds since "
      "January 1, 1970.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_END_TIME ),
      std::make_shared< klv_uint_format >( 8 ),
      "End Time",
      "Time of the most recent observation of the entity. Microseconds since "
      "January 1, 1970.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_BOUNDARY_SERIES ),
      std::make_shared< klv_0903_location_series_format >(),
      "Boundary Series",
      "Set of vertices that specify a minimum bounding area or volume. "
      "Encloses full extent of VMTI detections for the entity.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_ALGORITHM ),
      std::make_shared< klv_string_format >(),
      "Algorithm",
      "Name or description of the algorith or method used to create or "
      "maintain object movement reports or predictions.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_CONFIDENCE ),
      std::make_shared< klv_uint_format >( 1 ),
      "Confidence Level",
      "Estimation of the certainty or correctness of VMTI movement "
      "detections. Larger values indicate greater confidence.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_SYSTEM_NAME ),
      std::make_shared< klv_string_format >(),
      "VMTI System Name",
      "Name or description of the VMTI system producing the targets.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_VERSION ),
      std::make_shared< klv_uint_format >(),
      "VMTI LS Version",
      "Version of MISB ST 0903 used as the source standard when encoding this "
      "set.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_SOURCE_SENSOR ),
      std::make_shared< klv_string_format >(),
      "VMTI Source Sensor",
      "Name of VMTI source sensor. Examples: 'EO Nose', 'EO Zoom (DLTV)'.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACKER_NUM_TRACK_POINTS ),
      std::make_shared< klv_uint_format >(),
      "Number of Track Points",
      "Number of coordinates which describe the history of VMTI detections "
      "described by `Track History Series`.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_ITEM_SERIES ),
      std::make_shared< klv_0903_vtrackitem_series_format >(),
      "Track Item Series",
      "Series of track item metadata values.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTRACK_ONTOLOGY_SERIES ),
      std::make_shared< klv_0903_ontology_series_format >(),
      "Ontology Series",
      "A series of ontology local sets.",
      { 0, 1 } }, };

  return lookup;
}

// ----------------------------------------------------------------------------
klv_0903_vtrack_local_set_format
::klv_0903_vtrack_local_set_format()
  : klv_local_set_format{ klv_0903_vtrack_set_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_0903_vtrack_local_set_format
::description() const
{
  return "vtrack local set of " + length_description();
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
