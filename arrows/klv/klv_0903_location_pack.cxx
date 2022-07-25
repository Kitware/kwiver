// This file is part of KWIVER, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/kwiver/blob/master/LICENSE for details.

/// \file
/// Implementation of the KLV 0903 location pack parser.

#include "klv_0903_location_pack.h"

#include <arrows/klv/klv_util.h>

#include <vital/logger/logger.h>

#include <iomanip>

namespace kv = kwiver::vital;

namespace kwiver {

namespace arrows {

namespace klv {

namespace {

// ----------------------------------------------------------------------------
klv_0903_sigma_pack
klv_0903_read_sigma_pack( klv_read_iter_t& data, size_t length )
{
  auto const tracker = track_it( data, length );
  klv_0903_sigma_pack result;
  result.east = klv_read_imap( 0.0, 650.0, data, tracker.verify( 2 ) );
  result.north = klv_read_imap( 0.0, 650.0, data, tracker.verify( 2 ) );
  result.up = klv_read_imap( 0.0, 650.0, data, tracker.verify( 2 ) );
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0903_write_sigma_pack( klv_0903_sigma_pack const& value,
                           klv_write_iter_t& data, size_t length )
{
  auto const tracker = track_it( data, length );
  klv_write_imap( value.east, 0.0, 650.0, data, tracker.verify( 2 ) );
  klv_write_imap( value.north, 0.0, 650.0, data, tracker.verify( 2 ) );
  klv_write_imap( value.up, 0.0, 650.0, data, tracker.verify( 2 ) );
}

// ----------------------------------------------------------------------------
size_t
klv_0903_sigma_pack_length()
{
  return 6;
}

// ----------------------------------------------------------------------------
klv_0903_rho_pack
klv_0903_read_rho_pack( klv_read_iter_t& data, size_t length )
{
  auto const tracker = track_it( data, length );
  klv_0903_rho_pack result;
  result.east_north = klv_read_imap( -1.0, 1.0, data, tracker.verify( 2 ) );
  result.east_up = klv_read_imap( -1.0, 1.0, data, tracker.verify( 2 ) );
  result.north_up = klv_read_imap( -1.0, 1.0, data, tracker.verify( 2 ) );
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0903_write_rho_pack( klv_0903_rho_pack const& value,
                         klv_write_iter_t& data, size_t length )
{
  auto const tracker = track_it( data, length );
  klv_write_imap( value.east_north, -1.0, 1.0, data, tracker.verify( 2 ) );
  klv_write_imap( value.east_up, -1.0, 1.0, data, tracker.verify( 2 ) );
  klv_write_imap( value.north_up, -1.0, 1.0, data, tracker.verify( 2 ) );
}

// ----------------------------------------------------------------------------
size_t
klv_0903_rho_pack_length()
{
  return 6;
}

} // namespace

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_sigma_pack const& value )
{
  os    << "{ "
        << "east: " << value.east << ", "
        << "north: " << value.north << ", "
        << "up: " << value.up
        << " }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0903_sigma_pack,
  &klv_0903_sigma_pack::east,
  &klv_0903_sigma_pack::north,
  &klv_0903_sigma_pack::up
  )

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_rho_pack const& value )
{
  os    << "{ "
        << "east-north: " << value.east_north << ", "
        << "east-up: " << value.east_up << ", "
        << "north-up: " << value.north_up
        << " }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0903_rho_pack,
  &klv_0903_rho_pack::east_north,
  &klv_0903_rho_pack::east_up,
  &klv_0903_rho_pack::north_up
  )

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_location_pack const& value )
{
  os    << "{ "
        << "latitude: " << value.latitude << ", "
        << "longitude: " << value.longitude << ", "
        << "altitude: " << value.altitude << ", "
        << "sigma: " << value.sigma << ", "
        << "rho: " << value.rho
        << " }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0903_location_pack,
  &klv_0903_location_pack::latitude,
  &klv_0903_location_pack::longitude,
  &klv_0903_location_pack::altitude,
  &klv_0903_location_pack::sigma,
  &klv_0903_location_pack::rho
  )

// ----------------------------------------------------------------------------
std::ostream&
operator<<( std::ostream& os, klv_0903_velocity_pack const& value )
{
  os    << "{ " << std::setprecision( 17 )
        << "east: " << value.east << ", "
        << "north: " << value.north << ", "
        << "up: " << value.up << ", "
        << "stddev: " << value.sigma << ", "
        << "correlation: " << value.rho
        << " }";
  return os;
}

// ----------------------------------------------------------------------------
DEFINE_STRUCT_CMP(
  klv_0903_velocity_pack,
  &klv_0903_velocity_pack::east,
  &klv_0903_velocity_pack::north,
  &klv_0903_velocity_pack::up,
  &klv_0903_velocity_pack::sigma,
  &klv_0903_velocity_pack::rho
  )

// ----------------------------------------------------------------------------
klv_0903_location_pack_format
::klv_0903_location_pack_format()
  : klv_data_format_< klv_0903_location_pack >{ 0 }
{}

// ----------------------------------------------------------------------------
std::string
klv_0903_location_pack_format
::description() const
{
  return "location pack of " + length_description();
}

// ----------------------------------------------------------------------------
klv_0903_location_pack
klv_0903_location_pack_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_0903_location_pack result;
  result.latitude = klv_read_imap( -90.0, 90.0, data, tracker.verify( 4 ) );
  result.longitude = klv_read_imap( -180.0, 180.0, data, tracker.verify( 4 ) );
  result.altitude =
    klv_read_imap( -900.0, 19000.0, data, tracker.verify( 2 ) );
  if( tracker.remaining() >= klv_0903_sigma_pack_length() )
  {
    result.sigma =
      klv_0903_read_sigma_pack(
        data, tracker.verify( klv_0903_sigma_pack_length() ) );
  }
  if( tracker.remaining() >= klv_0903_rho_pack_length() )
  {
    result.rho =
      klv_0903_read_rho_pack(
        data, tracker.verify( klv_0903_rho_pack_length() ) );
  }
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0903_location_pack_format
::write_typed( klv_0903_location_pack const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_write_imap( value.latitude, -90.0, 90.0, data, tracker.verify( 4 ) );
  klv_write_imap( value.longitude, -180.0, 180.0, data, tracker.verify( 4 ) );
  klv_write_imap( value.altitude, -900.0, 19000.0, data, tracker.verify( 2 ) );
  if( value.sigma )
  {
    klv_0903_write_sigma_pack(
      *value.sigma, data, tracker.verify( klv_0903_sigma_pack_length() ) );
    if( value.rho )
    {
      klv_0903_write_rho_pack(
        *value.rho, data, tracker.verify( klv_0903_rho_pack_length() ) );
    }
  }
  else if( value.rho )
  {
    LOG_WARN( kv::get_logger( "klv" ),
              "cannot write rho pack without preceding sigma pack" );
  }
}

// ----------------------------------------------------------------------------
size_t
klv_0903_location_pack_format
::length_of_typed( klv_0903_location_pack const& value ) const
{
  return 4 + 4 + 2 +
         ( value.sigma ? klv_0903_rho_pack_length() : 0 ) +
         ( value.rho ? klv_0903_sigma_pack_length() : 0 );
}

// ----------------------------------------------------------------------------
klv_0903_velocity_pack_format
::klv_0903_velocity_pack_format()
  : klv_data_format_< klv_0903_velocity_pack >{ 0 }
{}

// ----------------------------------------------------------------------------
std::string
klv_0903_velocity_pack_format
::description() const
{
  return "velocity/acceleration pack of " + length_description();
}

// ----------------------------------------------------------------------------
klv_0903_velocity_pack
klv_0903_velocity_pack_format
::read_typed( klv_read_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_0903_velocity_pack result;
  result.east = klv_read_imap( -900.0, 900.0, data, tracker.verify( 2 ) );
  result.north = klv_read_imap( -900.0, 900.0, data, tracker.verify( 2 ) );
  result.up = klv_read_imap( -900.0, 900.0, data, tracker.verify( 2 ) );
  if( tracker.remaining() >= klv_0903_sigma_pack_length() )
  {
    result.sigma =
      klv_0903_read_sigma_pack(
        data, tracker.verify( klv_0903_sigma_pack_length() ) );
  }
  if( tracker.remaining() >= klv_0903_rho_pack_length() )
  {
    result.rho =
      klv_0903_read_rho_pack(
        data, tracker.verify( klv_0903_rho_pack_length() ) );
  }
  return result;
}

// ----------------------------------------------------------------------------
void
klv_0903_velocity_pack_format
::write_typed( klv_0903_velocity_pack const& value,
               klv_write_iter_t& data, size_t length ) const
{
  auto const tracker = track_it( data, length );
  klv_write_imap( value.east, -900.0, 900.0, data, tracker.verify( 2 ) );
  klv_write_imap( value.north, -900.0, 900.0, data, tracker.verify( 2 ) );
  klv_write_imap( value.up, -900.0, 900.0, data, tracker.verify( 2 ) );
  if( value.sigma )
  {
    klv_0903_write_sigma_pack(
      *value.sigma, data, tracker.verify( klv_0903_sigma_pack_length() ) );
    if( value.rho )
    {
      klv_0903_write_rho_pack(
        *value.rho, data, tracker.verify( klv_0903_rho_pack_length() ) );
    }
  }
  else if( value.rho )
  {
    LOG_WARN( kv::get_logger( "klv" ),
              "cannot write rho pack without preceding sigma pack" );
  }
}

// ----------------------------------------------------------------------------
size_t
klv_0903_velocity_pack_format
::length_of_typed( klv_0903_velocity_pack const& value ) const
{
  return 2 + 2 + 2 +
         ( value.sigma ? klv_0903_rho_pack_length() : 0 ) +
         ( value.rho ? klv_0903_sigma_pack_length() : 0 );
}

} // namespace klv

} // namespace arrows

} // namespace kwiver
