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

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

constexpr size_t checksum_packet_length = 4;
std::vector< uint8_t > const checksum_header = { KLV_0903_CHECKSUM, 2 };

} // namespace <anonymous>

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
      std::make_shared< klv_string_format >(),
      "VMTI System Name",
      "Name or description of the VMTI system producing the targets.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VERSION ),
      std::make_shared< klv_uint_format >(),
      "VMTI LS Version",
      "Version of MISB ST 0903 used as the source standard when encoding this "
      "set.",
      1 },
    { {},
      ENUM_AND_NAME( KLV_0903_NUM_TARGETS_DETECTED ),
      std::make_shared< klv_uint_format >(),
      "Total Number of Targets Detected",
      "Total number of targets detected in a frame.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_NUM_TARGETS_REPORTED ),
      std::make_shared< klv_uint_format >(),
      "Number of Targets Reported",
      "Number of targets reported following a culling process.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_FRAME_NUMBER ),
      std::make_shared< klv_uint_format >(),
      "Frame Number",
      "Frame number identifying detected targets.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_FRAME_WIDTH ),
      std::make_shared< klv_uint_format >(),
      "Frame Width",
      "Width of the Motion Imagery frame in pixels.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_FRAME_HEIGHT ),
      std::make_shared< klv_uint_format >(),
      "Frame Height",
      "Height of the Motion Imagery frame in pixels.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_SOURCE_SENSOR ),
      std::make_shared< klv_string_format >(),
      "VMTI Source Sensor",
      "Name of VMTI source sensor. Examples: 'EO Nose', 'EO Zoom (DLTV)'.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_HORIZONTAL_FOV ),
      std::make_shared< klv_imap_format >( 0.0, 180.0 ),
      "VMTI Horizontal FOV",
      "Horizonal field of view of sensor input to the VMTI process.",
      { 0, 1 } },
    { {},
      ENUM_AND_NAME( KLV_0903_VERTICAL_FOV ),
      std::make_shared< klv_imap_format >( 0.0, 180.0 ),
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
  : klv_local_set_format{ klv_0903_traits_lookup() }
{}

// ----------------------------------------------------------------------------
std::string
klv_0903_local_set_format
::description() const
{
  return "vmti local set of " + length_description();
}

// ----------------------------------------------------------------------------
uint32_t
klv_0903_local_set_format
::calculate_checksum( klv_read_iter_t data, size_t length ) const
{
  return klv_running_sum_16( checksum_header.begin(), checksum_header.end(),
                             klv_running_sum_16( data, data + length ),
                             length % 2 );
}

// ----------------------------------------------------------------------------
uint32_t
klv_0903_local_set_format
::read_checksum( klv_read_iter_t data, size_t length ) const
{
  if( length < checksum_packet_length )
  {
    VITAL_THROW( kv::metadata_buffer_overflow,
                 "packet too small; checksum is not present" );
  }
  data += length - checksum_packet_length;

  if( !std::equal( checksum_header.cbegin(), checksum_header.cend(), data ) )
  {
    VITAL_THROW( kv::metadata_exception,
                 "checksum header not present" );
  }
  data += checksum_header.size();

  return klv_read_int< uint16_t >( data, 2 );
}

// ----------------------------------------------------------------------------
void
klv_0903_local_set_format
::write_checksum( uint32_t checksum,
                  klv_write_iter_t& data, size_t max_length ) const
{
  if( max_length < checksum_packet_length )
  {
    VITAL_THROW( kv::metadata_buffer_overflow,
                 "writing checksum packet overflows data buffer" );
  }
  data = std::copy( checksum_header.cbegin(), checksum_header.cend(), data );
  klv_write_int( static_cast< uint16_t >( checksum ), data, 2 );
}

// ----------------------------------------------------------------------------
size_t
klv_0903_local_set_format
::checksum_length() const
{
  return checksum_packet_length;
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
