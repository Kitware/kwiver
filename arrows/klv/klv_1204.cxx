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

namespace {

// ----------------------------------------------------------------------------
klv_1204_uuid
klv_read_uuid( klv_read_iter_t& data, size_t max_length )
{
  if( max_length < 16 )
  {
    VITAL_THROW( kv::metadata_buffer_overflow,
                 "reading UUID overflows buffer" );
  }

  klv_1204_uuid result;
  std::copy_n( data, 16, result.bytes.begin() );
  data += 16;
  return result;
}

// ----------------------------------------------------------------------------
void
klv_write_uuid( klv_1204_uuid const& value, klv_write_iter_t& data,
                size_t max_length )
{
  if( max_length < 16 )
  {
    VITAL_THROW( kv::metadata_buffer_overflow,
                 "writing UUID overflows buffer" );
  }
  data = std::copy( value.bytes.begin(), value.bytes.end(), data );
}

// ----------------------------------------------------------------------------
size_t
klv_uuid_length()
{
  return 16;
}

} // namespace

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
// Prints like 0123-4567-89AB-CDEF-0123-4567-89AB-CDEF
std::ostream&
operator<<( std::ostream& os, klv_1204_uuid const& value )
{
  auto const flags = os.flags();

  for( size_t i = 0; i < value.bytes.size(); ++i )
  {
    os << std::hex << std::setfill( '0' ) << std::setw( 2 );
    if( i != 0 && i % 2 == 0 )
    {
      os << '-';
    }
    os << static_cast< unsigned int >( value.bytes[ i ] );
  }

  os.flags( flags );
  return os;
}

// ----------------------------------------------------------------------------
bool
operator==( klv_1204_uuid const& lhs, klv_1204_uuid const& rhs )
{
  return lhs.bytes == rhs.bytes;
}

// ----------------------------------------------------------------------------
bool
operator<( klv_1204_uuid const& lhs, klv_1204_uuid const& rhs )
{
  return lhs.bytes < rhs.bytes;
}

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
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
KWIVER_ALGO_KLV_EXPORT
bool
operator==( klv_1204_miis_id const& lhs, klv_1204_miis_id const& rhs )
{
  return
    lhs.version == rhs.version &&
    lhs.sensor_id_type == rhs.sensor_id_type &&
    lhs.platform_id_type == rhs.platform_id_type &&
    lhs.sensor_id == rhs.sensor_id &&
    lhs.platform_id == rhs.platform_id &&
    lhs.window_id == rhs.window_id &&
    lhs.minor_id == rhs.minor_id;
}

// ----------------------------------------------------------------------------
KWIVER_ALGO_KLV_EXPORT
bool
operator<( klv_1204_miis_id const& lhs, klv_1204_miis_id const& rhs )
{
  if( lhs.version < rhs.version )
  {
    return true;
  }
  if( lhs.version > rhs.version )
  {
    return false;
  }

  if( lhs.sensor_id_type < rhs.sensor_id_type )
  {
    return true;
  }
  if( lhs.sensor_id_type > rhs.sensor_id_type )
  {
    return false;
  }

  if( lhs.platform_id_type < rhs.platform_id_type )
  {
    return true;
  }
  if( lhs.platform_id_type > rhs.platform_id_type )
  {
    return false;
  }

  if( lhs.sensor_id < rhs.sensor_id )
  {
    return true;
  }
  if( lhs.sensor_id > rhs.sensor_id )
  {
    return false;
  }

  if( lhs.platform_id < rhs.platform_id )
  {
    return true;
  }
  if( lhs.platform_id > rhs.platform_id )
  {
    return false;
  }

  if( lhs.window_id < rhs.window_id )
  {
    return true;
  }
  if( lhs.window_id > rhs.window_id )
  {
    return false;
  }

  return lhs.minor_id < rhs.minor_id;
}

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
  auto const begin = data;
  auto const remaining_length = [ & ]() -> size_t {
                                  return length - std::distance( begin, data );
                                };

  // Single byte version number
  result.version =
    klv_read_int< uint8_t >( data,
                             std::min< size_t >( remaining_length(), 1 ) );

  // Single usage byte
  auto const usage =
    klv_read_int< uint8_t >( data,
                             std::min< size_t >( remaining_length(), 1 ) );

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
    result.sensor_id = klv_read_uuid( data, remaining_length() );
  }

  if( platform_id_present )
  {
    result.platform_id = klv_read_uuid( data, remaining_length() );
  }

  if( window_id_present )
  {
    result.window_id = klv_read_uuid( data, remaining_length() );
  }

  if( minor_id_present )
  {
    result.minor_id = klv_read_uuid( data, remaining_length() );
  }

  return result;
}

// ----------------------------------------------------------------------------
void
klv_1204_miis_id_format
::write_typed( klv_1204_miis_id const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const begin = data;
  auto const remaining_length = [ & ]() -> size_t {
                                  return length - std::distance( begin, data );
                                };

  // Single byte version number
  klv_write_int( value.version, data, std::min< size_t >(
                   remaining_length(), 1 ) );

  // Combine bit fields into single usage byte
  auto const usage = static_cast< uint8_t >(
    ( static_cast< uint8_t >( value.sensor_id_type ) << 5 ) |
    ( static_cast< uint8_t >( value.platform_id_type ) << 3 ) |
    ( static_cast< uint8_t >( value.window_id.has_value() ) << 2 ) |
    ( static_cast< uint8_t >( value.minor_id.has_value() ) << 1 ) );
  klv_write_int( usage, data, std::min< size_t >( remaining_length(), 1 ) );

  // Write UUIDs that are present
  if( value.sensor_id )
  {
    klv_write_uuid( *value.sensor_id, data, remaining_length() );
  }

  if( value.platform_id )
  {
    klv_write_uuid( *value.platform_id, data, remaining_length() );
  }

  if( value.window_id )
  {
    klv_write_uuid( *value.window_id, data, remaining_length() );
  }

  if( value.minor_id )
  {
    klv_write_uuid( *value.minor_id, data, remaining_length() );
  }
}

// ----------------------------------------------------------------------------
size_t
klv_1204_miis_id_format
::length_of_typed( klv_1204_miis_id const& value, size_t length_hint ) const
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
