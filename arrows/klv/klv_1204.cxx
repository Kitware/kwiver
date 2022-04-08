// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// \brief Implementation of the KLV 1204 parser.

#include "klv_1204.h"

#include <arrows/klv/klv_data_format.h>
#include <arrows/klv/klv_util.h>

#include <iomanip>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1204_device_id_type value )
{
  static std::string strings[ KLV_1204_DEVICE_ID_ENUM_END + 1 ] =
  {
    "None",
    "Managed",
    "Virtual",
    "Physical",
    "Unknown Device ID Type" };

  return os << strings[ std::min( value, KLV_1204_DEVICE_ID_ENUM_END ) ];
}

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_1204_miis_id const& value )
{
  os << "{ "
     << "version: "
     << static_cast< unsigned int >( value.version ) << ", "
     << "sensor id type: "
     << value.sensor_id_type << ", "
     << "sensor id: "
     << value.sensor_id << ", "
     << "platform id type: "
     << value.platform_id_type << ", "
     << "platform id: "
     << value.platform_id << ", "
     << "window id: "
     << value.window_id << ", "
     << "minor id: "
     << value.minor_id
     << " }";

  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_1204_miis_id,
  &klv_1204_miis_id::version,
  &klv_1204_miis_id::sensor_id_type,
  &klv_1204_miis_id::platform_id_type,
  &klv_1204_miis_id::sensor_id,
  &klv_1204_miis_id::platform_id,
  &klv_1204_miis_id::window_id,
  &klv_1204_miis_id::minor_id
)

// ----------------------------------------------------------------------------
klv_1204_miis_id_format
::klv_1204_miis_id_format()
  : klv_data_format_< klv_1204_miis_id >{ 0 }
{}

// ----------------------------------------------------------------------------
std::string
klv_1204_miis_id_format
::description() const
{
  return "MIIS ID of " + length_description();
}

// ----------------------------------------------------------------------------
klv_1204_miis_id
klv_1204_miis_id_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  klv_1204_miis_id result;
  auto const tracker = track_it( data, length );

  // Single byte version number
  result.version =
    klv_read_int< uint8_t >( data, tracker.verify( 1 ) );

  // Single usage byte
  auto const usage =
    klv_read_int< uint8_t >( data, tracker.verify( 1 ) );

  // Extract bit fields from usage byte
  result.sensor_id_type =
    static_cast< klv_1204_device_id_type >( ( usage & 0x60 ) >> 5 );
  result.platform_id_type =
    static_cast< klv_1204_device_id_type >( ( usage & 0x18 ) >> 3 );

  auto const sensor_id_present =
    result.sensor_id_type != KLV_1204_DEVICE_ID_TYPE_NONE;
  auto const platform_id_present =
    result.platform_id_type != KLV_1204_DEVICE_ID_TYPE_NONE;
  auto const window_id_present = static_cast< bool >( usage & 0x04 );
  auto const minor_id_present = static_cast< bool >( usage & 0x02 );

  // Read UUIDs based on usage
  if( sensor_id_present )
  {
    result.sensor_id = klv_read_uuid( data, tracker.remaining() );
  }

  if( platform_id_present )
  {
    result.platform_id = klv_read_uuid( data, tracker.remaining() );
  }

  if( window_id_present )
  {
    result.window_id = klv_read_uuid( data, tracker.remaining() );
  }

  if( minor_id_present )
  {
    result.minor_id = klv_read_uuid( data, tracker.remaining() );
  }

  return result;
}

// ----------------------------------------------------------------------------
void
klv_1204_miis_id_format
::write_typed( klv_1204_miis_id const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );

  // Single byte version number
  klv_write_int( value.version, data, std::min< size_t >(
                   tracker.remaining(), 1 ) );

  // Combine bit fields into single usage byte
  auto const usage = static_cast< uint8_t >(
    ( static_cast< uint8_t >( value.sensor_id_type ) << 5 ) |
    ( static_cast< uint8_t >( value.platform_id_type ) << 3 ) |
    ( static_cast< uint8_t >( value.window_id.has_value() ) << 2 ) |
    ( static_cast< uint8_t >( value.minor_id.has_value() ) << 1 ) );
  klv_write_int( usage, data, std::min< size_t >( tracker.remaining(), 1 ) );

  // Write UUIDs that are present
  if( value.sensor_id )
  {
    klv_write_uuid( *value.sensor_id, data, tracker.remaining() );
  }

  if( value.platform_id )
  {
    klv_write_uuid( *value.platform_id, data, tracker.remaining() );
  }

  if( value.window_id )
  {
    klv_write_uuid( *value.window_id, data, tracker.remaining() );
  }

  if( value.minor_id )
  {
    klv_write_uuid( *value.minor_id, data, tracker.remaining() );
  }
}

// ----------------------------------------------------------------------------
size_t
klv_1204_miis_id_format
::length_of_typed( klv_1204_miis_id const& value ) const
{
  // Version number byte + usage byte + 16 bytes for each UUID present
  return
    2 +
    ( value.sensor_id ? klv_uuid_length() : 0 ) +
    ( value.platform_id ? klv_uuid_length() : 0 ) +
    ( value.window_id ? klv_uuid_length() : 0 ) +
    ( value.minor_id ? klv_uuid_length() : 0 );
}

// ----------------------------------------------------------------------------
klv_uds_key
klv_1204_key()
{
  return { 0x060E2B3401010101, 0x0E01040503000000 };
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
