// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of the KLV 0806 parser.

#include "klv_0806.h"

#include "klv_0806_aoi_set.h"
#include "klv_0806_poi_set.h"
#include "klv_0806_user_defined_set.h"
#include "klv_checksum.h"

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

constexpr size_t checksum_packet_length = 6;
std::vector< uint8_t > const checksum_header = { KLV_0806_CHECKSUM, 4 };

} // namespace <anonymous>

// ----------------------------------------------------------------------------
klv_tag_traits_lookup const&
klv_0806_traits_lookup()
{
  static klv_tag_traits_lookup const lookup = {
    { {},
      ENUM_AND_NAME( KLV_0806_UNKNOWN ),
      std::make_shared< klv_blob_format >(),
      "Unknown Tag",
      "Unknown tag.",
      0 },
    { { 0x060E2B3401010101, 0x0E01020310000000 },
      ENUM_AND_NAME( KLV_0806_CHECKSUM ),
      std::make_shared< klv_uint_format >( 4 ),
      "Checksum",
      "Checksum used to detect errors within a ST 0806 packet.",
      0 },
    { { 0x060E2B3401010101, 0x0702010101050000 },
      ENUM_AND_NAME( KLV_0806_TIMESTAMP ),
      std::make_shared< klv_uint_format >( 8 ),
      "Timestamp",
      "Precision timestamp expressed in microseconds since the UNIX Epoch.",
      1 },
    { { 0x060E2B3401010101, 0x0E0101010A010000 },
      ENUM_AND_NAME( KLV_0806_PLATFORM_TRUE_AIRSPEED ),
      std::make_shared< klv_uint_format >( 2 ),
      "Platform True Airspeed",
      "True airspeed of the platform: indicated airspeed adjusted for "
      "temperature and altitude. Measured in meters per second.",
      1 },
    { { 0x060E2B3401010101, 0x0E0101010B010000 },
      ENUM_AND_NAME( KLV_0806_PLATFORM_INDICATED_AIRSPEED ),
      std::make_shared< klv_uint_format >( 2 ),
      "Platform Indicated Airspeed",
      "Indicated airspeed of the platform. Derived from Pitot tube and static "
      "pressure sensors. Measured in meters per second.",
      1 },
    { { 0x060E2B3401010101, 0x0E01010314000000 },
      ENUM_AND_NAME( KLV_0806_TELEMETRY_ACCURACY_INDICATOR ),
      std::make_shared< klv_blob_format >(),
      "Telemetry Accuracy Indicator",
      "Reserved for future use.",
      0 },
    { { 0x060E2B3401010101, 0x0E01010315000000 },
      ENUM_AND_NAME( KLV_0806_FRAG_CIRCLE_RADIUS ),
      std::make_shared< klv_uint_format >( 2 ),
      "Frag Circle Radius",
      "Size of fragmentation circle selected by the aircrew. Measured in "
      "meters.",
      1 },
    { { 0x060E2B3401010101, 0x0E01010309000000 },
      ENUM_AND_NAME( KLV_0806_FRAME_CODE ),
      std::make_shared< klv_uint_format >( 4 ),
      "Frame Code",
      "Counter runs at 60Hz.",
      1 },
    { { 0x060E2B3401010101, 0x0E01020303000000 },
      ENUM_AND_NAME( KLV_0806_VERSION_NUMBER ),
      std::make_shared< klv_uint_format >( 1 ),
      "UAS LS Version Number",
      "Version of MISB ST 0806 used as the source standard when encoding this "
      "local set.",
      1 },
    { { 0x060E2B3401010103, 0x0E01010119000000 },
      ENUM_AND_NAME( KLV_0806_VIDEO_DATA_RATE ),
      std::make_shared< klv_uint_format >( 4 ),
      "Video Data Rate",
      "Video data rate if digital, or analog FM. Measured in bits per second "
      "or Hertz.",
      1 },
    { { 0x060E2B3401010103, 0x04010B0100000000 },
      ENUM_AND_NAME( KLV_0806_DIGITAL_VIDEO_FILE_FORMAT ),
      std::make_shared< klv_string_format >(),
      "Digital Video File Format",
      "Video compression being used. Examples: MPEG2, MPEG4, H.264, Analog "
      "FM.",
      1 },
    { { 0x060E2B34020B0101, 0x0E0103010F000000 },
      ENUM_AND_NAME( KLV_0806_USER_DEFINED_LOCAL_SET ),
      std::make_shared< klv_0806_user_defined_set_format >(),
      "User Defined Local Set",
      "Local set of user-defined data items.",
      { 0, SIZE_MAX },
      &klv_0806_user_defined_set_traits_lookup() },
    { { 0x060E2B34020B0101, 0x0E0103010C000000 },
      ENUM_AND_NAME( KLV_0806_POI_LOCAL_SET ),
      std::make_shared< klv_0806_poi_set_format >(),
      "Point of Interest Local Set",
      "Local set with point-of-interest information.",
      { 0, SIZE_MAX },
      &klv_0806_poi_set_traits_lookup() },
    { { 0x060E2B34020B0101, 0x0E0103010D000000 },
      ENUM_AND_NAME( KLV_0806_AOI_LOCAL_SET ),
      std::make_shared< klv_0806_aoi_set_format >(),
      "Area of Interest Local Set",
      "Local set with area-of-interest information.",
      { 0, SIZE_MAX },
      &klv_0806_aoi_set_traits_lookup() },
    { { 0x060E2B3401010101, 0x0E0101030A000000 },
      ENUM_AND_NAME( KLV_0806_MGRS_ZONE ),
      std::make_shared< klv_uint_format >( 1 ),
      "MGRS Zone",
      "UTM Zone 01 through 60.",
      1 },
    { { 0x060E2B3401010101, 0x0E0101030B000000 },
      ENUM_AND_NAME( KLV_0806_MGRS_LATITUDE_BAND_GRID_SQUARE ),
      std::make_shared< klv_string_format >(),
      "MGRS Latitude Band and Grid Square",
      "First character is the alpha code for the latitude band. Second and "
      "third are the alpha code for the WGS84 grid square designator.",
      1 },
    { { 0x060E2B3401010101, 0x0E0101030C000000 },
      ENUM_AND_NAME( KLV_0806_MGRS_EASTING ),
      std::make_shared< klv_uint_format >( 3 ),
      "MGRS Easting",
      "Five-digit easting value in meters.",
      1 },
    { { 0x060E2B3401010101, 0x0E0101030D000000 },
      ENUM_AND_NAME( KLV_0806_MGRS_NORTHING ),
      std::make_shared< klv_uint_format >( 3 ),
      "MGRS Northing",
      "Five-digit northing value in meters.",
      1 },
    { { 0x060E2B3401010101, 0x0E0101030A010000 },
      ENUM_AND_NAME( KLV_0806_FRAME_CENTER_MGRS_ZONE ),
      std::make_shared< klv_uint_format >( 1 ),
      "Frame Center MGRS Zone",
      "UTM Zone 01 through 60.",
      1 },
    { { 0x060E2B3401010101, 0x0E0101030B010000 },
      ENUM_AND_NAME( KLV_0806_FRAME_CENTER_MGRS_LATITUDE_BAND_GRID_SQUARE ),
      std::make_shared< klv_string_format >(),
      "Frame Center MGRS Latitude Band and Grid Square",
      "First character is the alpha code for the latitude band. Second and "
      "third are the alpha code for the WGS84 grid square designator.",
      1 },
    { { 0x060E2B3401010101, 0x0E0101030C010000 },
      ENUM_AND_NAME( KLV_0806_FRAME_CENTER_MGRS_EASTING ),
      std::make_shared< klv_uint_format >( 3 ),
      "Frame Center MGRS Easting",
      "Five-digit easting value in meters.",
      1 },
    { { 0x060E2B3401010101, 0x0E0101030D010000 },
      ENUM_AND_NAME( KLV_0806_FRAME_CENTER_MGRS_NORTHING ),
      std::make_shared< klv_uint_format >( 3 ),
      "Frame Center MGRS Northing",
      "Five-digit northing value in meters.",
      1 }, };

  return lookup;
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0806_tag tag )
{
  return os << klv_0806_traits_lookup().by_tag( tag ).name();
}

// ----------------------------------------------------------------------------
klv_0806_local_set_format
::klv_0806_local_set_format() : klv_local_set_format{ klv_0806_traits_lookup() }
{
}

// ----------------------------------------------------------------------------
std::string
klv_0806_local_set_format
::description() const
{
  return "ST 0806 local set of " + length_description();
}

// ----------------------------------------------------------------------------
uint32_t
klv_0806_local_set_format
::calculate_checksum( klv_read_iter_t data, size_t length ) const
{
  return klv_crc_32_mpeg( checksum_header.begin(), checksum_header.end(),
                          klv_crc_32_mpeg( data, data + length ) );
}

// ----------------------------------------------------------------------------
uint32_t
klv_0806_local_set_format
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

  return klv_read_int< uint32_t >( data, 4 );
}

// ----------------------------------------------------------------------------
void
klv_0806_local_set_format
::write_checksum( uint32_t checksum,
                  klv_write_iter_t& data, size_t max_length ) const
{
  if( max_length < checksum_packet_length )
  {
    VITAL_THROW( kv::metadata_buffer_overflow,
                 "writing checksum packet overflows data buffer" );
  }
  data = std::copy( checksum_header.cbegin(), checksum_header.cend(), data );
  klv_write_int( checksum, data, 4 );
}

// ----------------------------------------------------------------------------
size_t
klv_0806_local_set_format
::checksum_length() const
{
  return checksum_packet_length;
}

// ----------------------------------------------------------------------------
klv_uds_key
klv_0806_key()
{ return { 0x060E2B34020B0101, 0x0E01030102000000 }; }

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0806_poi_type tag )
{
  static std::string strings[ KLV_0806_POI_AOI_TYPE_ENUM_END - 1 ] = {
    "Friendly",
    "Hostile",
    "Target",
    "Unknown" };

  auto index = static_cast< size_t >( tag ) - 1u;
  if( index >= KLV_0806_POI_AOI_TYPE_ENUM_END - 1 )
  {
    index = KLV_0806_POI_AOI_TYPE_UNKNOWN;
  }
  return os << strings[ index ];
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
