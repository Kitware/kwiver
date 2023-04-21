// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0903 parser.

#include "klv_0903.h"

#include <arrows/klv/klv_0903_algorithm_set.h>
#include <arrows/klv/klv_0903_ontology_set.h>
#include <arrows/klv/klv_0903_vtarget_pack.h>
#include <arrows/klv/klv_1204.h>
#include <arrows/klv/klv_checksum.h>
#include <arrows/klv/klv_series.hpp>
#include <arrows/klv/klv_string.h>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_tag tag )
{
  return os << klv_0903_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
klv_uds_key
klv_0903_key()
{
  return { 0x060E2B34020B0101, 0x0E01030306000000 };
}

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0903_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0903_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown",
      "Unknown tag.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0903_CHECKSUM ),
      std::make_shared< klv_uint_format >( 2 ),
      "Checksum",
      "Checksum used to detect errors within a ST 0903 packet.",
      0 },
    { {},
      ENUM_AND_NAME( KLV_0903_PRECISION_TIMESTAMP ),
      std::make_shared< klv_uint_format >(),
      "Precision Timestamp",
      "Microseconds since January 1st, 1970.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VMTI_SYSTEM_NAME ),
      std::make_shared< klv_utf_8_format >( 32 ),
      "VMTI System Name",
      "Name or description of the VMTI system producing the targets.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VERSION ),
      std::make_shared< klv_uint_format >( klv_length_constraints{ 1, 2 } ),
      "VMTI LS Version",
      "Version of MISB ST 0903 used as the source standard when encoding this "
      "set.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0903_NUM_TARGETS_DETECTED ),
      std::make_shared< klv_uint_format >( klv_length_constraints{ 1, 3 } ),
      "Total Number of Targets Detected",
      "Total number of targets detected in a frame.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_NUM_TARGETS_REPORTED ),
      std::make_shared< klv_uint_format >( klv_length_constraints{ 1, 3 } ),
      "Number of Targets Reported",
      "Number of targets reported following a culling process.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_FRAME_NUMBER ),
      std::make_shared< klv_uint_format >( klv_length_constraints{ 1, 3 } ),
      "Frame Number",
      "Frame number identifying detected targets.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_FRAME_WIDTH ),
      std::make_shared< klv_uint_format >( klv_length_constraints{ 1, 3 } ),
      "Frame Width",
      "Width of the Motion Imagery frame in pixels.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_FRAME_HEIGHT ),
      std::make_shared< klv_uint_format >( klv_length_constraints{ 1, 3 } ),
      "Frame Height",
      "Height of the Motion Imagery frame in pixels.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_SOURCE_SENSOR ),
      std::make_shared< klv_utf_8_format >( 128 ),
      "VMTI Source Sensor",
      "Name of VMTI source sensor. Examples: 'EO Nose', 'EO Zoom (DLTV)'.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_HORIZONTAL_FOV ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ 0.0, 180.0 }, 2 ),
      "VMTI Horizontal FOV",
      "Horizonal field of view of sensor input to the VMTI process.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VERTICAL_FOV ),
      std::make_shared< klv_imap_format >(
        kv::interval< double >{ 0.0, 180.0 }, 2 ),
      "VMTI Vertical FOV",
      "Vertical field of view of sensor input to the VMTI process.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_MIIS_ID ),
      std::make_shared< klv_1204_miis_id_format >(),
      "MIIS ID",
      "A Motion Imagery Identification System Core Identifier conformant with "
      "MISB ST 1204.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VTARGET_SERIES ),
      std::make_shared< klv_0903_vtarget_series_format >(),
      "VTarget Series",
      "A series of VTarget packs.",
      { 0, 1 },
      &klv_0903_vtarget_pack_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0903_ALGORITHM_SERIES ),
      std::make_shared< klv_0903_algorithm_series_format >(),
      "Algorithm Series",
      "A series of algorithm local sets.",
      { 0, 1 },
      &klv_0903_algorithm_set_traits_lookup() },
    { {},
      ENUM_AND_NAME( KLV_0903_ONTOLOGY_SERIES ),
      std::make_shared< klv_0903_ontology_series_format >(),
      "Ontology Series",
      "A series of ontology local sets.",
      { 0, 1 },
      &klv_0903_ontology_set_traits_lookup() },
  };

  return lookup;
}

// ----------------------------------------------------------------------------
klv_0903_local_set_format
::klv_0903_local_set_format()
  : klv_local_set_format{ klv_0903_traits_lookup() },
    m_checksum_format{ { KLV_0903_CHECKSUM, 2 } }
{}

// ----------------------------------------------------------------------------
std::string
klv_0903_local_set_format
::description_() const
{
  return "ST0903 VMTI LS";
}

// ----------------------------------------------------------------------------
klv_checksum_packet_format const*
klv_0903_local_set_format
::checksum_format() const
{
  return &m_checksum_format;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
